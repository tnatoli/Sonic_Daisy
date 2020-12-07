#include "daisysp.h"
#include "daisy_petal.h"
#include "terrarium.h"

#include <string>
#include <cmath>

// 48kHz sample rate (48,000 samples per second)
//   a max delay of 1 second would be 48000*1.f, 2 seconds would be 48000*2.f, etc.
#define MAX_DELAY static_cast<size_t>(48000 * 3.f) 
//   ^You should make this use samplerate instead of the hardcoded value.....but how?

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal petal;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[4];
static Tone toneLP; // this is for a Tone object, Tone is just a low-pass filter
static ATone toneHP; // this is for a ATone object, ATone is just a high-pass filter
static Balance bal; // this is for a Balance object, which will correct for volume drop from the filters
static CrossFade  cfade; //this is for a CrossFade object, which we will to blend the wet/dry and maintain a constant volume 

// Declare some global variable so all functions can see them
float feedback;
float maxFeedback = 1.0f; // Max value of feedback knob, maxFeedback=1 -> forever repeats but no selfoscillation, values over 1 allow runaway feedback fun
float maxRunawayFeedback = 1.5f; // This is what the feedback will go to when the second footswitch is pressed
bool feedbackRunaway = false;
float drywet_ratio = 0.5f; // drywet_ratio=0.0 is effect off
float tone_val =0.0f;

struct Delay
{
    DelayLine<float, MAX_DELAY> *delay;
    float                        currentDelay;
    float                        delayTarget;
    float                        feedback;

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f); // This smoothes out the delay when you turn the delay control?
        delay->SetDelay(currentDelay);
        float read = delay->Read();
        delay->Write((feedback * read) + in);
        return read;
    }
};

Delay delays[4];// This creates a delay structure to store delay parameters
// All these Params will be controlled by Pots
Parameter delayParams[4];
Parameter feedbackParam;
Parameter mixParam;
Parameter toneParam;

// These are the cutoff freqs for the high and low pass filters
float tone_freqHP;
float tone_freqLP;

// Each delay head will be turned on/off independently
bool delayOn[4];

// Use the LED object
Led led1, led2;

bool  passThruOn; // When this is true 'bypass' is on

void ProcessControls();

static void AudioCallback(float **in, float **out, size_t size)
{
    ProcessControls();//This makes sure all the moveable parameters are up to date (knobs, switches, leds, etc)
    toneHP.SetFreq(tone_freqHP);
    toneLP.SetFreq(tone_freqLP);
    
    for(size_t i = 0; i < size; i++)
	{
	    float final_mix = 0;
	    float all_delay_signals = 0;
	    float pre_filter_delay_signals = 0; 
		
	    if(passThruOn){ // This if statement makes the Daisy only process the input through the delays when the pedal is turned 'on'
		// Do below is the pedal is in 'bypass' mode
		out[0][i] = in[0][i]; // in[0][i] is the input audio, out[0][i] is the output audio
		all_delay_signals = 0; 
	    }
	    else{
		// create the delayed signal for each of the 4 delays
		for(int d = 0; d < 4; d++)
		    if(delayOn[d]){
			delays[d].feedback = feedback;
			all_delay_signals += delays[d].Process(in[0][i]);
		    }

		// Save the pre-filtered delay signals as a reference volume
		pre_filter_delay_signals = all_delay_signals;

		// Filter the delay signals with a LP and HP based on the tone knob
		all_delay_signals = toneHP.Process(all_delay_signals);
		all_delay_signals = toneLP.Process(all_delay_signals);
		// This 'balances' (I think compresses) the pre & post filter delayed signal to make up for volume loss due to the filter
		all_delay_signals = bal.Process(all_delay_signals, pre_filter_delay_signals*(1.f-tone_val/1.5f));
		//** at extreme HP levels this introduces noise for the lower notes since it has to raise the volume a lot to make up for the low frew loss
		//     I added the tone_val factor here to account for this noise a bit and to lower the repeat volume at the extremes of the tone pot
		
		// Use a crossfade object to maintain a constant power while creating the delayed/raw audio mix
		cfade.SetPos(drywet_ratio);
		final_mix = cfade.Process(in[0][i], all_delay_signals);
		out[0][i] = final_mix; // this sends 'final_mix' to the (left) audio
	    }
	}
}


void InitDelays(float samplerate)
{
    for(int i = 0; i < 4; i++)
    {
        //Initialize delays
        delayMems[i].Init();
        delays[i].delay = &delayMems[i];
	delays[i].feedback = 0; 
        //delay times: - just one knob controls all delay times
	//      (delay times for 1-3 are set as fractions of this knob value in ProcessControls below)
	delayParams[i].Init(petal.knob[Terrarium::KNOB_1], samplerate * .05, MAX_DELAY * 1.0,Parameter::LINEAR); 
    }
    
    mixParam.Init(petal.knob[Terrarium::KNOB_2], 0.0, 1.0, Parameter::LINEAR);  // mix is knob 2
    feedbackParam.Init(petal.knob[Terrarium::KNOB_3], 0.0, maxFeedback, Parameter::LINEAR);  // feedback is knob 3
}

void InitTone(float samplerate)
{
   // Initialize the Tone object
    toneHP.Init(samplerate);
    toneLP.Init(samplerate);
    toneParam.Init(petal.knob[Terrarium::KNOB_4], -1.0f, 1.0f, Parameter::LINEAR); // This knob value will be converted later in ProcessControls to a frequency for the High/Low Pass filter
}

void InitLeds(void)
{
    //Initialize the leds - these are using LED objects
    led1.Init(petal.seed.GetPin(Terrarium::LED_1),false);
    led2.Init(petal.seed.GetPin(Terrarium::LED_2),false);
    // The 'Terrarium::LED_1' (and similar for the knobs) references the terrarium.h which defines which GPIO pins
    //     are associated with which knobs, switches, & LEDs

}

int main(void)
{
    float samplerate;
    petal.Init(); // Initialize hardware (daisy seed, and petal)
    samplerate = petal.AudioSampleRate();
    /************************************************************************************************************/
    /************************************************************************************************************/
    // If you have a noise line at 1kHz adjusting the number blow from 48 to 12 may help reduce the noise line. 
    petal.SetAudioBlockSize(12);////////Adjust the blocksize - setting this to 12 moves the noise line to 4kHz and reduces it
    /************************************************************************************************************/
    /************************************************************************************************************/

    // Initialize the delay lines
    InitDelays(samplerate);

    // Initialize the Tone object
    InitTone(samplerate);

    // Initializes the Balance object
    bal.Init(samplerate);
    
    // set params for CrossFade object
    cfade.Init();
    cfade.SetCurve(CROSSFADE_CPOW);
    //This sets to crossfade to maintain constant power, which will maintain a constant volume as we go from full dry to full wet on the mix knob

    // Initialize the leds
    InitLeds();
    
    passThruOn = true;// This starts the pedal in the 'off' (or delay bypassed) position

    petal.StartAdc();
    petal.StartAudio(AudioCallback);

    while(1)
    {
        dsy_system_delay(6);
    }
}

void ProcessControls()
{
    petal.UpdateAnalogControls();
    petal.DebounceControls();
    led1.Update();
    // The brightnes of led2 tells us the fractional difference between the current feedback and the feedback knob setting
    //   The led turns off a little early because there is a voltage offset for an led to turn on, but this will vary  bit by led
    led2.Set((feedback-feedbackParam.Value())/(maxRunawayFeedback-feedbackParam.Value()));
    led2.Update();

    /////////////////////////////////////////
    //knobs
    //  Below each delay line has its own delay value
    for(int i = 0; i < 4; i++)
	    //The (i+0.25-i*0.75) just sets the delay intervals to 1/4,1/2/,3/4,1.0 for i=0,1,2,3
	    delays[i].delayTarget = (i+0.25-i*0.75)*delayParams[i].Process();
    drywet_ratio = mixParam.Process();
    //feedback is processed below, since it will depend on what footswitch 2

    // use a potentiameter for a tone control - toneParam sweeps from -1 to 1
    tone_val = toneParam.Process();
    if (tone_val<0.0f){ // left half of pot HP off, LP on
	tone_freqHP = 0;
    	tone_freqLP = 5000.0f*(powf(10,2*tone_val))+100.f;//This is a more complex function just to make the taper nice and smooth, the filter turned on too fast when linear
    }
    else{// right half of pot HP on, LP off 
	tone_freqHP = 5000.0f*powf(10,2.f*tone_val-2);//This is a more complex function just to make the taper nice and smooth, the filter turned on too fast when linear
	tone_freqLP = 1000000.0f;// just something very high so the filter is not killing any actual guitar sound
    }
    
    /////////////////////////////////////////
    //Delay Switches
    //     - The .Pressed() function below counts an 'ON' switch as pressed.
    //     - Each delay is controlled by it's own switch
    int switches[4] = {Terrarium::SWITCH_1, Terrarium::SWITCH_2, Terrarium::SWITCH_3, Terrarium::SWITCH_4};
    for(int i=0; i<4; i++)
	delayOn[i] = petal.switches[switches[i]].Pressed();

    /////////////////////////////////////////
    //footswitch 1 = 'bypass' 
    if(petal.switches[Terrarium::FOOTSWITCH_1].RisingEdge()) // ON/OFF footswitch
	{
	    passThruOn = !passThruOn;
	    led1.Set(passThruOn ? 0.0f : 1.0f);
	    // Above is an imbedded 'if' statement. If 'passThruOn=true' then set the led value to 0, if 'passThruOn=false' then set led value to 1.0
	}

    /////////////////////////////////////////
    // footswitch 2 = RUNAWAY FEEDBACK
    if(petal.switches[Terrarium::FOOTSWITCH_2].Pressed()) //runaway feedback footswitch
	feedbackRunaway = true;
    else
	feedbackRunaway = false;

    // **I think the ramp-up and ramp-down times feel a bit better, but if they varied based on the current length of the delay time it might be better
    //    Right now he noise floor rises pretty high during runawayFeedback for longer delays
    if(feedbackRunaway)
	//If FS_2 is pressed, start bringing the feedback up to maxRunawayFeedback
	fonepole(feedback, maxRunawayFeedback, 0.002f); //decrease the number for a faster ramp & vice-versa
    else
	//Bring the feedback to be equal to the knob value, if FS_2 was not pressed this will aleady be the case
	fonepole(feedback, feedbackParam.Process(), 0.001f); //decrease the number for a faster ramp & vice-versa
}

