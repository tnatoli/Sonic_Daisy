#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

#define NUM_WAVEFORMS 4

using namespace daisy;
using namespace daisysp;
using namespace terrarium;

DaisyPetal petal;
static Oscillator osc,osc1,osc2,osc3; // osc1,osc2,osc3 are for making chords
static WhiteNoise noise;
static CrossFade cfade,mixfade;

Led led1;
Parameter freqParam;
Parameter waveParam;
Parameter octaveParam;
Parameter volMixParam;
Parameter noiseMixParam;
Parameter intervalParam;

uint8_t waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE,
};

static float freq,volMix,noiseMix;
static int   waveform, octave, interval_choice;
float        sig, noise_sig, final_mix;
bool  passThruOn; // When this is true 'bypass' is on
bool discreteNotes, chord;

void ProcessControls();

static void AudioCallback(AudioHandle::InputBuffer in,
			  AudioHandle::OutputBuffer out,
                          size_t                                size)
{
    ProcessControls();
    // Audio Loop
    for(size_t i = 0; i < size; i += 2)
    {
        // Process
        sig = osc.Process();
	if(chord){
		sig+=osc1.Process();
		sig+=osc2.Process();
		sig+=osc3.Process();
	    }

	noise_sig = noise.Process();
	cfade.SetPos(noiseMix);
	mixfade.SetPos(volMix);
	final_mix = 0.1*cfade.Process(sig,noise_sig);//the volume of the noise & oscillators are loud, the prefactor just brings it down a bit
	float orig0 = in[0][i];
    	float orig1 = in[1][i];

	if(!passThruOn){
	    out[0][i]  = mixfade.Process(orig0,final_mix);
	    out[1][i]  = mixfade.Process(orig1,final_mix);
	}
	else{
	    out[0][i] = in[0][i];
	    out[1][i] = in[1][i];
	}
    }
}

void InitSynth(float samplerate)
{
    // Initiate Oscillators
    osc.Init(samplerate);
    osc1.Init(samplerate);
    osc2.Init(samplerate);
    osc3.Init(samplerate);

    // Initiate Noise 
    noise.Init();
    
    waveform = 0;
    octave   = 0;
}

int main(void)
{
    float samplerate;

    // Init everything
    petal.Init();
    petal.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    samplerate = petal.AudioSampleRate();
    petal.SetAudioBlockSize(1);
    InitSynth(samplerate);
    led1.Init(petal.seed.GetPin(Terrarium::LED_1),false);

    // Initialize & set params for CrossFade object
    cfade.Init();
    cfade.SetCurve(CROSSFADE_CPOW);
    mixfade.Init();
    mixfade.SetCurve(CROSSFADE_CPOW);
    
    // Init Params
    volMixParam.Init(petal.knob[Terrarium::KNOB_1], 0, 1, Parameter::LINEAR);
    octaveParam.Init(petal.knob[Terrarium::KNOB_2], 0, 1, Parameter::LINEAR);
    //freqParam.Init(petal.knob[Terrarium::KNOB_3], 10, 127, Parameter::LINEAR);
    freqParam.Init(petal.knob[Terrarium::KNOB_3], 0, 1, Parameter::LINEAR);
    noiseMixParam.Init(petal.knob[Terrarium::KNOB_4], 0, 1, Parameter::LINEAR);
    intervalParam.Init(petal.knob[Terrarium::KNOB_5], 0, 3.99, Parameter::LINEAR);
    waveParam.Init(petal.knob[Terrarium::KNOB_6], 0, 4, Parameter::LINEAR);

    
    passThruOn = true;// This starts the pedal in the 'off' (or delay bypassed) position
    
    // start callbacks
    petal.StartAdc();
    petal.StartAudio(AudioCallback);

    while(1) {}
}

void ProcessControls(void)
{
    int int1=0, int2=0, int3=0;
    petal.ProcessAnalogControls();
    petal.ProcessDigitalControls();
    led1.Update();
    waveform = DSY_CLAMP(waveParam.Process(), 0, NUM_WAVEFORMS);//waveform of the lfo
    osc.SetWaveform(waveforms[waveform]);
    osc.SetAmp(1.f);
    
    // Octave knob, 6 octaves available
    //octave = DSY_CLAMP(octaveParam.Process()*6,0,6);
    octave = DSY_CLAMP(octaveParam.Process()*6,0,6);
    
    // convert MIDI to frequency and multiply by octave size
    //freq = mtof(DSY_CLAMP(freqParam.Process() + (octave * 12),10,127));
    freq = freqParam.Process();
    if(discreteNotes){
	freq = int(freq*12);
	//freq = DSY_CLAMP(freq*12,0,12);
    }
    else
	freq = freq*11;
    freq+= 28 + (octave*12);
    osc.SetFreq(mtof(freq));
    interval_choice = floor(intervalParam.Process());
    if (interval_choice == 0){//major
	int1 = 4;
	int2 = 7;
	int3 = 12;}
    else if(interval_choice == 1){//minor
	int1 = 3;
	int2 = 7;
	int3 = 12;}
    else if(interval_choice == 2){//major7
	int1 = 4;
	int2 = 7;
	int3 = 11;}
    else if(interval_choice == 3){//minor7
	int1 = 3;
	int2 = 7;
	int3 = 10;}
    osc1.SetFreq(mtof(freq+ int1 ));
    osc2.SetFreq(mtof(freq+ int2 ));
    osc3.SetFreq(mtof(freq+ int3 ));
    
    // Output mix
    volMix = volMixParam.Process();
    noiseMix = noiseMixParam.Process();
    
    /////////////////////////////////////////
    //footswitch 1 = 'bypass' 
    if(petal.switches[Terrarium::FOOTSWITCH_1].RisingEdge()) // ON/OFF footswitch
	{
	    passThruOn = !passThruOn;
	    led1.Set(passThruOn ? 0.0f : 1.0f);
	    // Above is an imbedded 'if' statement. If 'passThruOn=true' then set the led value to 0, if 'passThruOn=false' then set led value to 1.0
	}
    //toggle 1 = discrete notes
    if(petal.switches[Terrarium::SWITCH_1].Pressed()) // ON/OFF toggle
	discreteNotes = true;
    else
	discreteNotes = false;
    //toggle 2 = chords
    if(petal.switches[Terrarium::SWITCH_2].Pressed()) // ON/OFF toggle
	chord = true;
    else
	chord = false;
}
