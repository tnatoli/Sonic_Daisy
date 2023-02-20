20230120 Sonic Explorer

# Sample Noises
Compiled with the DaisyExamples git hash: 

# Description
An oscillator with selectable shape, chord voicing, and white noise. 

This is set up to use the Terrarium hardware system from PedalPCB.

I typically just use this to test other pedals, but it could be used a basic noisemaker iteself. 

# Control

| Control | Description | Comment |
| --- | --- | --- |
| Ctrl 1 | Mix | Sets the dry/wet amount for the outputs, dry is whatever is going into the pedal, wet is the oscillator + noise |
| Ctrl 2 | Octave | Sets the octave range of the oscillator |
| Ctrl 3 | Frequency | Set the frequency of the oscillator note inside the octave |
| Ctrl 4 | Noise | Adjusts the mix of oscillator signal to white noise, more white noise is clockwise|
| Ctrl 5 | Chord | Changes the chord voicing, left to right goes from major, minor, major7, minor7 |
| Ctrl 6 | Waveshape | Changes the waveshape of the oscillator, left to righ goes from sine, triangle, sawtooth, square |
| SW 1 | Quantize | Quantizes the oscillator frequency selection (Cntrl 3) |
| SW 2 | Chords | Adds notes to the oscillator to create the chords selected by Ctrl 5 |
| FS 1 | Bypass/Active | Turns the osciallor & noise on/off |
| LED 1 | Bypass/Active Indicator |Illuminated when effect is set to Active |
| Audio In 1 | Audio input | |
| Audio In 2 | Audio input | |
| Audio Out 1 | Mix Out | |
| Audio Out 2 | Mix Out | |


![Conrtrols Image](simple_noises.png?raw=true)



------

TO DO:
 -  Add in some vibrato with one of the toggle switches. 
 -  Add an arpeggiator. 

VERSION HISTORY:
 - 20230220 v1 - Initial release. 

