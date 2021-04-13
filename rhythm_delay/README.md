20210308 Sonic Explorer

# Rhythmic Delay v2.0
Compiled with the DaisyExamples git hash: a6de44acedf955da11c7974c3578253719b63554

# Description
Selectrable 4 Head delay with Echorec timing

This is set up to use the Terrarium hardware system from PedalPCB.

The delay spacings are set like the Echorc (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1).
All delay times are adjusted with Ctrl 1 (current max 3 second).
Feedback for all 4 delays controlled by Ctrl 3. 
The four delay signals are mixed with the dry signal with Cntrl 2 and sent out to the left channel.
The Tone knob (Ctrl 4) only affects the delays and is off at noon. Clockwise from noon turns on a high pass filter, counterclockwise from noon tuns on a low pass filter.
The Age knob (Ctrl 5) adds modulation to the repeats mimicing tape/drum imperfections for the first half, and then adds degradation-like artifacts to the modulation on the second half.
The Swell knob (Ctrl 6) mixes in a faux reverb by sending the delayed signals back into a 4 head delay with all heads active. This faux reverb extends beyond the normal repeats.
The right footswtich sets the feedback to infinite repeats without self-oscillating, this produces a sound-on-sound effect.
The right LED shows how much the current feedback level is above the feedback pot setting. 

# Control

| Control | Description | Comment |
| --- | --- | --- |
| Ctrl 1 | Delay Time| All delays will be relative to this value; 0.25, 0.5, 0.75, 1.0|
| Ctrl 2 | Mix | Set the dry/wet amount for the outputs, wet here is the delays |
| Ctrl 3 | Feedback | Set the amount of repeats for all delays|
| Ctrl 4 | Tone | left = HP filter = darker, right = HP filter =brighter, this affects repeats only|
| Ctrl 5 | Age | First half adds some modulation to mimic tape/drum age, second half adds degredation |
| Ctrl 6 | Swell | Adjusts the amount of a faux reverb created when all 4 heads are on |
| SW 1 - 4 | Delays On/Off | Each switch turns a delay head on/off |
| FS 1 | Bypass/Active | Changes the status of the pedal from audio being passed through the pedal unaffected to having delays added |
| FS 2 | Secondary Feedback | Sets the feedback at 0.95 to produce a kind of sound-on-sound where repeats take a very long time to decay |
| LED 1 | Bypass/Active Indicator |Illuminated when effect is set to Active |
| LED 2 | Delay Time Indicator | Shows the delay time of head 4 |
| Audio In 1 | Audio input | |
| Audio Out 1 | Mix Out | |

------

TO DO:
 -  Completely erase the delayed signals from memory when the effect is bypassed so there are not tails when the delay is activated.
 -  Add the ability to save what the secondary footswitch does and switch between options; erase mode, sound-on-sound mode, runaway feedback, rubberneck, etc.

POTENTIAL MODS:
 -  Adjust the modulation on the Age knob with your own preferences (the amplitude of modulation is an easy adjustment).
 -  Change what the second footswitch can do, tap tempo? mechanically halt the spinning tape/drum? change delay time for space noises?
 -  Use the second light to show the delay time.
 -  Trade out the Swell effect/control with a real reverb.
 -  Imporve the Age degradation by using a bit crusher or white noise or something else. 

VERSION HISTORY:
 - 20201130 v1
 - 20210308 v2 - No more 1kHz whine, added Age & Swell controls. 
