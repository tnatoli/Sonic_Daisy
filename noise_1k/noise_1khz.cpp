#include "daisysp.h"
#include "daisy_petal.h"
#include "terrarium.h"

#include <string>

#define MAX_DELAY static_cast<size_t>(48000 * 1.f)

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal petal;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delayMems[3];
float feedback = 0.25f;
float wetdry = 0.5f;

struct Delay
{
    DelayLine<float, MAX_DELAY> *delay;
    float                        currentDelay;
    float                        delayTarget;

    float Process(float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        delay->SetDelay(currentDelay);
        float read = delay->Read();
        delay->Write((feedback * read) + in);
        return read;
    }
};

Delay     delays[3];
Parameter delayParams[3];
Parameter feedbackParam;
Parameter mixParam;

dsy_gpio led1;

// int   drywet;
bool  passThruOn;

void ProcessControls();

static void AudioCallback(float **in, float **out, size_t size)
{
    ProcessControls();

    for(size_t i = 0; i < size; i++)
    {
	if (passThruOn)
	    out[0][i] = out[1][i] = in[0][i];
	else
	    {
		float mix = 0;
		//update delayline with feedback
		for(int d = 0; d < 3; d++)
			mix += delays[d].Process(in[0][i]);
		mix       = (wetdry * mix) / 3.0f + (1.0f - wetdry) * in[0][i];
		out[0][i] = out[1][i] = mix;
	    }
    }
}

void InitDelays(float samplerate)
{
    for(int i = 0; i < 3; i++)
    {
        //Init delays
        delayMems[i].Init();
        delays[i].delay = &delayMems[i];
        //3 delay times
        int knob;
	knob = 0.5f;
        switch(i) {
        case 0:
            knob = 0.25f;//Terrarium::KNOB_1;
            break;
        case 1:
            knob = 0.5f;//Terrarium::KNOB_2;
            break;
        case 2:
            knob = 0.75f;//Terrarium::KNOB_3;
            break;
        }
        delayParams[i].Init(petal.knob[knob],
                       samplerate * .05,
                       MAX_DELAY,
                       Parameter::LOGARITHMIC);
    }
    feedbackParam.Init(petal.knob[Terrarium::KNOB_4], 0.0, 1.0, Parameter::LINEAR); 
    mixParam.Init(petal.knob[Terrarium::KNOB_5], 0.0, 1.0, Parameter::LINEAR); 
}

int main(void)
{
    float samplerate;
    petal.Init(); // Initialize hardware (daisy seed, and petal)
    samplerate = petal.AudioSampleRate();
/************************************************************************************************************/
/************************************************************************************************************/
    petal.SetAudioBlockSize(48);////////Adjust the blocksize - setting this to 12 moves the noise line to 4kHz and reduces it
/************************************************************************************************************/
/************************************************************************************************************/
    InitDelays(samplerate);

    passThruOn = false;

    petal.StartAdc();
    petal.StartAudio(AudioCallback);

    led1.pin = petal.seed.GetPin(22);
    led1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    led1.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&led1);

    while(1)
    {
        // Update Pass thru
        dsy_gpio_write(&led1, passThruOn ? 0.0f : 1.0f);
        dsy_system_delay(6);
    }
}

void ProcessControls()
{
    petal.UpdateAnalogControls();
    petal.DebounceControls();

    //knobs
    for(int i = 0; i < 3; i++)
    {
        delays[i].delayTarget = delayParams[i].Process();
    }

    feedback    = feedbackParam.Process();
    wetdry = mixParam.Process();

    //footswitch
    if(petal.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        passThruOn = !passThruOn;
    }
}
