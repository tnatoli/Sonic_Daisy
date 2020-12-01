#20201130 Sonic Explorer

# Description
Selectrable 4 Head delay with Echorec timing

TO DO:
 - When a note is played and then the delays are engaged the note still repeats, this is unexpected behavior and should be corrected.
 - runaway feedback for longer delay times seems to raise the noise floor up way too much
 
-----
This is set up to use the Terrarium hardware system from PedalPCB.

The delay spacings are set like the Echorc (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1).
Longest delay currently set to 3 seconds for delay head 4.
All delay times are set with with Pot 1 (current max 3 second). Feedback for all 4 delays controlled by Pot 3.
The Four delays are mixed with the dry signal and sent to the left channel, wet/dry mix is Pot 2. 
The Tone knob (pot 4) only affects the delays and is off at noon. Clockwise from noon turns on a high pass filter, counterclockwise from noon tuns on a low pass filter.
The right footswitch ramps the feedback to runaway (self-oscillation), the right led shows how much the current feedback level is above the feedback pot setting.

# Control

| Control | Description | Comment |
| --- | --- | --- |
| Ctrl 1 | Delay Time | All delays will be a fraction of this value |
| Ctrl 2 | Dry/Wet | Set the dry/wet amount for the output |
| Ctrl 3 | Feedback| For all 4 delays |
| Ctrl 4 | Tone | left = HP_filter = darker, right = HP_filter = brighter - these only affect repeats|
| SW 1 - 4 | Delays On/Off | Each switch turns a delay head on/off |
| FS 1 | Passthru | led1 illuminated when effect is set to ON |
| FS 2 | Runaway Feedback | led2 illuminated when ramping feedback up and down|
| Audio In 1 | Audio input | |
| Audio Out 1 | Mix Out | |

## Things to modify/add:

 - Use one of the extra knobs to control has fast the runaway feedback ramps up and down.
 - Add analog mojo to the repeats
   - Add 'wow and flutter' by adding imperfections into the repeats
   - Degrade the repeats to mimic an old tape or magnetic drum
   
