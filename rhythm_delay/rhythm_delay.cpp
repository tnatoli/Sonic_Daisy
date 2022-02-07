////// Rhythm Delay v2.1 - 20220206 - Written by Sonic Explorer //////
#include "daisysp.h"
#include "daisy_petal.h"
#include "terrarium.h"

#include <string>
#include <cmath>

// 48kHz sample rate (48,000 samples per second)
//   a max delay of 1 second would be 48000*1.f, 2 seconds would be 48000*2.f, etc.
#define MAX_DELAY static_cast<size_t>(48000 * 3.f)

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal petal;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[4];
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS swellMems[4];

Switch switch_r,switch_l;

static Tone toneLP; // this is for a Tone object, Tone is just a low-pass filter
static ATone toneHP; // this is for a ATone object, ATone is just a high-pass filter
static Balance bal; // this is for a Balance object, which will correct for volume drop from the filters
static CrossFade cfade; //this is for a CrossFade object, which we will to blend the wet/dry and maintain a constant volume
static Oscillator osc,osc2; //These are for the 'age' or modulation on the delays
static Oscillator osc_delay; //This is for LED2 to show the delay time
static Compressor comp;

bool dac_output = false; // make this true if you want to use the dac output instead of the default leds
float mod_out,mod_out2; // these are what is sent to the dac_output

// Declare some global variable so all functions can see them
float feedback = 0;
float maxFeedback = 1.2f; // Max value of feedback knob, maxFeedback=1 -> forever repeats but no selfoscillation, values over 1 allow runaway feedback fun
/////////// Change the secondaryFeedback to 0.95f if you want a 'Sound on Sound' feel, try setting it to 0.0f if you want to use it as a kind of erase button to minimize repeats, I like it at 1.5f for a runaway feedback when pressed.
float secondaryFeedback = 0.95f; // This is what the feedback will go to when the right/second footswitch is pressed
bool feedbackSecondary = false;
float drywet_ratio = 0.5f; // drywet_ratio=0.0 is effect off
float tone_val =0.0f, swell_val=0.0f;
float samplerate, osc_delay_val=0;
float mod_osc,mod_osc2;

float rand_now=0,rand_val=0;
bool R_ON,L_ON;

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
Delay swells[4];// This creates a delay structure for the 'swell' delays

// All these Params will be controlled by Pots
Parameter delayParam;
Parameter feedbackParam;
Parameter mixParam;
Parameter toneParam;
Parameter swellParam;
Parameter ageParam;

// Each delay head will be turned on/off independently
bool delayOn[4];

// Use the LED object
Led led1, led2;

bool  passThruOn; // When this is true 'bypass' is on

void ProcessControls(int part);//this has to be declared above the AudioCallback so it knows it is a function since it is defined below the AudioCallback.

int call_counter = 0;//This counts how many times audiocallback has been ran.
int process_counter = 0;//this keeps track of how many times the same part of procescontrol is ran. 
static void AudioCallback(AudioHandle::InputBuffer in,
			  AudioHandle::OutputBuffer out,
			  //AudioHandle::InterleavingInputBuffer  in,
                          //AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
                          //float **in, float **out, size_t size)
{
    call_counter+=1;
    int n_cases = 4;//the total amount of 'parts' ProcessControl is split into
    const int m_val = 2; //this is how many audiocallbacks between case calls
    int n_calls = n_cases*m_val;

    switch (call_counter%(n_calls))
	{//ProcessControls makes sure all the moveable parameters are up to date (knobs, switches, leds, etc)
	    // ProcessControls is split into 4 sub-parts to spread out the processing over different AudioCallbacks runs
	    //  I do not know if it was necessary to split ProcessControls into 4, but I got some artifacts when calling the full ProcessControls every callback
	case 0*m_val:
	    ProcessControls(1);
	    process_counter+=1;
	    break;
	case 1*m_val: ProcessControls(2);
	    break;
	case 2*m_val: ProcessControls(3);
	    break;
	case 3*m_val: ProcessControls(4);
	    break;
	}
	
    for(size_t i = 0; i < size; i++)
	{
	    float final_mix = 0;
	    float all_delay_signals = 0;
	    float swell_signals = 0;
	    float pre_filter_delay_signals = 0;
	    osc_delay_val = osc_delay.Process();
	    
	    if(passThruOn){ // This if statement makes the Daisy only process the input through the delays when the pedal is turned 'on'
		// Do below if the pedal is in 'bypass' mode
		out[0][i] = in[0][i]; // in[0][i] is the input audio, out[0][i] is the output audio
		out[1][i] = in[0][i]; // Something is odd with the audio_Out channels on the the Seedv1.1, so we send the audio in to both audio outs. 
		// The for statment below takes the feedback to 0 when the pedal is off, which *almost* clears the repeats, but not if the pedal is turned off and on again quickly. 
		for(int d = 0; d < 4; d++){
		    delays[d].feedback = 0;
		    delays[d].Process(0);
		    swells[d].feedback = feedback;
		    swells[d].Process(0);
		}
	    }
	    else{
		// calculate the oscillators for the Age control
		mod_osc = osc.Process();//This is the base sine wave
		mod_osc2 = osc2.Process();//This is the square wave for glitches

		// create the delayed signal for each of the 4 delays
		for(int d = 0; d < 4; d++)
		    if(delayOn[d]){
			delays[d].feedback = feedback;
			all_delay_signals += delays[d].Process(in[0][i]);
		    }
		// sends the delays signal through 4 delay heads that are always on for a faux reverb 'swell' effect
		for(int d = 0; d < 4; d++){
		    if(feedback<0.2f) /// This will make the swells adhere to the feedback knob at low feedback values
			swells[d].feedback = feedback;
		    else
			swells[d].feedback = 0.2f;
		    if(d==0)
			swell_signals += swells[d].Process(all_delay_signals)*0.5;
		    else
			swell_signals += swells[d].Process(swell_signals)*0.5;
		}

		//// Apply a bit of compression to the swells -- there is lots of room for the compression settings to be made better
		swell_signals = comp.Process(swell_signals);
		//swell_signals = comp.Process(swell_signals,all_delay_signals);//maybe sidechain this to the all_delay_singals? - I tried this but it sounded less natural to me. 

		// Add the swells to the normal delay head signals
		all_delay_signals += swell_signals*(swell_val*swell_val)/2;//two factor of swell_val and the 1/2 here just seemed to feel right for the amount
		
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
		//final_mix = cfade.Process(in[i], all_delay_signals);
		float orig = in[0][i]; // I don't exactly know why I have to do this, but something with the pointing is not behaving with cfade
		final_mix = cfade.Process(orig, all_delay_signals);
		out[0][i] = final_mix; // this sends 'final_mix' to the (left) audio
		out[1][i] = final_mix; // Something is odd with the audio_Out channels on the the Seedv1.1, so we send the effected audio to both audio outs. 
	    }
	    if(dac_output){
		petal.seed.dac.WriteValue(DacHandle::Channel::TWO, (mod_out+1)*1000);
		petal.seed.dac.WriteValue(DacHandle::Channel::ONE, (mod_out2+1)*1000);
	    }
	}
}

void InitDelays(float samplerate)
{ //Initialize all the delays & swells
    for(int i = 0; i < 4; i++)
    { 
        delayMems[i].Init();
        delays[i].delay = &delayMems[i];
	delays[i].feedback = 0;
	swellMems[i].Init();
        swells[i].delay = &swellMems[i];
	swells[i].feedback = 0; 
       //delay times: - just one knob controls all delay times
       //    (delay times for 1-3 are set as fractions of this knob value in ProcessControls below)
	
    }
    //delayParam.Init(petal.knob[Terrarium::KNOB_1], samplerate * .1, MAX_DELAY * 1.0,Parameter::LINEAR);
    delayParam.Init(petal.knob[Terrarium::KNOB_1], -1, 1.0,Parameter::LINEAR);
    mixParam.Init(petal.knob[Terrarium::KNOB_2], 0.0, 1.0, Parameter::LINEAR);  // mix is knob 2
    feedbackParam.Init(petal.knob[Terrarium::KNOB_3], 0.0, maxFeedback, Parameter::LINEAR);  // feedback is knob 3
}

void InitSwell(float samplerate)
{ // Initilize the compression and knob for the swell
    swellParam.Init(petal.knob[Terrarium::KNOB_6], 0.0, 1, Parameter::LINEAR);  // swell is knob 6
    comp.Init(samplerate);
    comp.SetThreshold(-40.0f);
    comp.SetRatio(10.0f);
    comp.SetAttack(0.5f);
    comp.SetRelease(0.5f);
}

void InitAge(float samplerate)
{ // Initilize all the things for the Age control
    ageParam.Init(petal.knob[Terrarium::KNOB_5], 0.0, 1, Parameter::LINEAR);//LOGARITHMIC);//LINEAR);  // age is knob 5
    osc.Init(samplerate);
    osc.SetWaveform(osc.WAVE_SIN);//TRI);
    osc.SetFreq(2.0f);
    osc.SetAmp(1.0f);
    osc2.Init(samplerate);
    osc2.SetWaveform(osc2.WAVE_SQUARE);    //osc2.SetWaveform(osc.WAVE_SAW);
    osc2.SetFreq(2.f);
    osc2.SetAmp(1.0f);
}

void InitTone(float samplerate)
{   // Initialize the Tone object
    toneHP.Init(samplerate);
    toneLP.Init(samplerate);
    toneParam.Init(petal.knob[Terrarium::KNOB_4], -1.0f, 1.0f, Parameter::LINEAR); // This knob value will be converted later in ProcessControls to a frequency for the High/Low Pass filter
}

void InitWaveOut()
{    // This uses the default pins of 29 & 30 for the DAC (GPIO22 & GPIO23) to output the waveforms
    //   These are the same pins as the Terrarium leds...so you cannot use both at the same time.
    DacHandle::Config cfg;
    cfg.bitdepth   = DacHandle::BitDepth::BITS_12;
    cfg.buff_state = DacHandle::BufferState::ENABLED;
    cfg.mode       = DacHandle::Mode::POLLING;
    cfg.chn        = DacHandle::Channel::BOTH;
    petal.seed.dac.Init(cfg);
}

void InitLeds(void)
{
    //Initialize the leds - these are using LED objects
    led1.Init(petal.seed.GetPin(Terrarium::LED_1),false);
    led2.Init(petal.seed.GetPin(Terrarium::LED_2),false);
    // The 'Terrarium::LED_1' (and similar for the knobs) references the terrarium.h which defines which GPIO pins
    //     are associated with which knobs, switches, & LEDs
    // The oscilator is being initiated here because it shows the delay time. 
    osc_delay.Init(samplerate);
    osc_delay.SetWaveform(osc.WAVE_SIN);
    osc_delay.SetFreq(2.0f);
    osc_delay.SetAmp(1.0f);
}

void InitExSwitches(void)// Setup the extra top switches
{/// I have to do this to use the extra switches I added to the top of my Terrarium build
    switch_l.Init(petal.seed.GetPin(27),petal.seed.AudioCallbackRate(),Switch::TYPE_TOGGLE,Switch::POLARITY_NORMAL,Switch::PULL_UP);
    switch_r.Init(petal.seed.GetPin(28),petal.seed.AudioCallbackRate(),Switch::TYPE_TOGGLE,Switch::POLARITY_NORMAL,Switch::PULL_UP);
    // a reminder you have to use switch_l.Debounch() & switch_r.Debounch() for the extra switches to actually work
}

float get_rand(void)//returns a random value between 0 and 1.0
{
    return rand()/(float)RAND_MAX -0.5;
}

int main(void)
{
    petal.Init(); // Initialize hardware (daisy seed, and petal)
    samplerate = petal.AudioSampleRate();
    petal.SetAudioBlockSize(1);////////Adjust the blocksize 

    // Setup the DACs:
    if(dac_output) // make dac_output=true in the program header to use the DAC outputs
	InitWaveOut();
    else    // Initialize the leds since you cannot use both the leds & dac on the Terrarium
        InitLeds();
    
    // Initialize the delay lines
    InitDelays(samplerate);
    
    // Initialize the Tone object
    InitTone(samplerate);
    
    // Initializes the Balance object
    bal.Init(samplerate);
    
    // Initializes the age functions/params
    InitAge(samplerate);

    // Initializes the swell functions/params
    InitSwell(samplerate);

    // Initialize & set params for CrossFade object
    cfade.Init();
    cfade.SetCurve(CROSSFADE_CPOW);
    //This sets to crossfade to maintain constant power, which will maintain a constant volume as we go from full dry to full wet on the mix knob

    //This is an example of what is required to add extra switches
    if(false)
	InitExSwitches();
    
    passThruOn = true;// This starts the pedal in the 'off' (or delay bypassed) position

    petal.StartAdc();
    petal.StartAudio(AudioCallback);

    while(1)
    {
	//   dsy_system_delay(6);
    }
}

void ProcessControls(int part)
{

    if(part==1){
	petal.ProcessAnalogControls();
	led1.Update();
	led2.Update();
	
	/////// Below is for led2 to show if knob feedback is different than the current feedback, this is useful for runaway feedback or sound-on-sound
	///////      .....but i found it was introducing some noise - so I changed led2 to show the delay time
	//////             - The led noise seems to only happen when some audio is going through the pedal and when something other than just true or false is feed to the led
	// The brightnes of led2 tells us the fractional difference between the current feedback and the feedback knob setting
	//   The led turns off a little early because there is a voltage offset for an led to turn on, but this will vary  bit by led
	//led2.Set((feedback-feedbackParam.Value())/(secondaryFeedback-feedbackParam.Value()));
	//led2.Update();

	// footswitch 2 (the right one)
	////// SECONDARY FEEDBACK -- this needs to be in the same 'part' as the ProcessAnalogControls() because it needs to be continually updated for the 
	if(petal.switches[Terrarium::FOOTSWITCH_2].Pressed()) // secondary feedback footswitch
	    feedbackSecondary = true;
	else
	    feedbackSecondary = false;
	// **I think the ramp-up and ramp-down times feel a bit better, but if they varied based on the current length of the delay time it might be better
	//    Right now he noise floor rises pretty high during secondaryFeedback for longer delays
	if(feedbackSecondary)
	    //If FS_2 is pressed, start bringing the feedback up to secondaryFeedback
	    fonepole(feedback, secondaryFeedback, 0.002f); //decrease the number for a faster ramp & vice-versa
	else
	    //Bring the feedback to be equal to the knob value, if FS_2 was not pressed this will aleady be the case
	    fonepole(feedback, feedbackParam.Process(), 0.001f); //decrease the number for a faster ramp & vice-versa
    }
    if(part==2){
	/////////////////////////////////////////
	//knobs
	float mod_total;
	float time_long;
	float age_val = ageParam.Process();

	time_long = delayParam.Process();//this is just the knob value from -1 to 1.
	time_long = MAX_DELAY*(0.1+0.9*(powf(time_long,3)+1)/2);//this now converts the knob value to the delay time in samples, 1 sample is 1/samplerate seconds.
	// The funny function with the powf and 0.1+0.9*(etc) is to get a knob taper where it starts at 0.1*MAX_DELAY, goes to 1*MAX_DELAY and spends most of the knobs rotation on the middle delay values. 

	osc_delay.SetFreq(samplerate/time_long);// this is set to the delaytime in Hz of the longest delay
	led2.Set(osc_delay_val>0.9); //led2 is set to show the delay time of head 4, the longest delay. ---- comment out this line if you want to turn off the delay time indicator
	
	/////// This next section all builds the Age modulation///////-------------------------------------------------
	// get a random value that updates every roughly 0.1*delay time, but with some randomness in how often one is selected
	if(process_counter%((int)(time_long/10))==(int)(get_rand()*time_long*0.05))
	    rand_val = get_rand();
	// Smooth out the random values to make them less jarring
	fonepole(rand_now,rand_val,1/(48000.0f*1/10.0f));//when the denominator is smaller the interpolation takes longer

	osc.SetFreq(0.25*samplerate/time_long);// samplerate/time_long should be the delaytime in Hz of the longest delay (head 4)
	osc2.SetFreq(0.17*samplerate/time_long);
	float max_amp = 0.02;//Make this number bigger if you want more modulation in the Age control ---- this is the modulation/Age magic number.
	float amp_val;
	if(age_val<0.5)//This maxes out the age_val at noon on the knob, so >Noon only addes degregation
	    amp_val = age_val*max_amp;
	else
	    amp_val = max_amp/2;
	// We will slowly build up the shape of the modulation for the Age parameter, comment out any of the lines below the first one to remove that component of the modulation
	mod_total = mod_osc;//This is the base sine wave
	mod_total *=(1+0.2*(mod_osc2+0.5));//This adds a small sq wave for some abrupt changes
	mod_total *= (1+(rand_now-0.5)*0.6);//This adds a slowly changing random value, pretty subtle right now
	mod_total *= amp_val; // This makes the amplitude of the modulation obey the value of the age knob

	// Add the degredation to the Age knob when it is above noon
	// ---this is kind of 'code-bending', we are changing hte delay time really fast and allowing it to inject artifacts into the delayed sound on purpose.
	if(age_val>0.5 && age_val <0.6)
	    mod_total += (powf(.6,3)/0.1)*(age_val-.5)*get_rand()/10; 
	if(age_val >0.6)
	    mod_total += powf(age_val,3)*get_rand()/10;
	
	//Below is for sending the mod signal to the dac_outputs to look at the waveform
	if(dac_output){
	    mod_out = mod_total/amp_val;
	    mod_out2 = mod_osc;
	}
	/////// End of building the Age (it gets added to the delay times just below)///////--------------------------------

	//  Below each delay line has its own delay value
	for(int i = 0; i < 4; i++)
	    //The (i+0.25-i*0.75) just sets the delay intervals to 1/4,1/2/,3/4,1.0 for i=0,1,2,3
	    //delays[i].delayTarget = (i+0.25-i*0.75)*time_long; //this is NO Age (modulation)
	    delays[i].delayTarget = (i+0.25-i*0.75)*time_long*(1+mod_total); //this also adds the Age to the swell delays
	
	drywet_ratio = mixParam.Process();
	//feedback is processed above, since it will depend on what footswitch 2

	// Below are the delays for the 'swell' reverb like sound
	for(int i = 0; i < 4; i++)
	    swells[i].delayTarget = (i+0.25-i*0.75)*time_long;//*(1+get_rand()*0.01);

    }//ends part 2
    
    if(part==3){
	// These are the cutoff freqs for the high and low pass filters
	float tone_freqHP;
	float tone_freqLP;

	// swell parameter updating
	swell_val = powf(swellParam.Process(),1/1.5);
	
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
	toneHP.SetFreq(tone_freqHP);
	toneLP.SetFreq(tone_freqLP);
    }
    if(part==4){
	//	petal.ProcessAnalogControls();
	petal.ProcessDigitalControls();
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

	// These are for the extra switches I added to my Terrarium build:
	if(false)
	    {
		switch_r.Debounce();
		switch_l.Debounce();
		L_ON = switch_l.Pressed();
		R_ON = switch_r.Pressed();
	    }
    }
}

