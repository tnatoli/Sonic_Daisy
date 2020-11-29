#20201128 Sonic Explorer

# Description
Selectrable 4 Head delay with Echorec timing

TO DO:

-----
This is set up to use the Terrarium hardware system from PedalPCB.

The delay spacings are set like the Echorc (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1).
Single delay time set with Pot 1 (current max 3 second). Feedback for all 4 delays controlled by Pot 3. 
The Four delays are mixed with the dry signal for both right and left channels (only the left mono channel is available with Terrarium).
The Tone knob (pot 4) only affects the delays and is off at noon. Clockwise from noon turns on a high pass filter, counterclockwise from noon tuns on a low pass filter

# Control

| Control | Description | Comment |
| --- | --- | --- |
| Ctrls 1 | Delay Time
| Ctrl 2 | Dry/Wet | Set the dry/wet amount for the outputs |
| Ctrl 3 | Feedback for all delays|
| Ctrl 4 | Tone | left = HP filter = darker, right = HP filter =brighter - affects repeats only|
| SW 1 - 4 | Delays On/Off | Each switch turns a delay head on/off |
| FS 1 | Passthru | led1 illuminated when effect is set to ON |
| FS 2 | Runaway Feedback | led1 illuminated when ramping feedback up and down|
| Audio In 1 | Audio input | |
| Audio Out 1 & Out 2| Mix Out | |

