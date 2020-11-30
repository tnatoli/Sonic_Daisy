#20201130 Sonic Explorer

# Description
Selectrable 4 Head delay with Echorec timing

TO DO:
- When a note is played and then the delays are engaged the note still repeats, this is unexpected behavior and should be corrected.

-----
This is set up to use the Terrarium hardware system from PedalPCB.

The delay spacings are set like the Echorc (1/16 note, 1/8 note, dotted 1/8 note, 1/4 note - or 0.25,0.5,0.75,1).
Longest delay currently set to 3 seconds for delay head 4.
All delay times are set with with Pot 1 (current max 3 second). Feedback for all 4 delays controlled by Pot 3. 
The Four delays are mixed with the dry signal and sent to the left channel, wet/dry mix is Pot 2. 

# Control

| Control | Description | Comment |
| --- | --- | --- |
| Ctrls 1 | Delay Time | All delays will be a fraction of this value |
| Ctrl 2 | Dry/Wet | Set the dry/wet amount output |
| Ctrl 3 | Feedback| For all 4 delays |
| SW 1 - 4 | Delays On/Off | Each switch turns a delay head on/off |
| FS 1 | Passthru | led1 illuminated when effect is set to ON |
| Audio In 1 | Audio input | |
| Audio Out 1 | Mix Out | |

## Quick things to modify/add:

 - Raise maxFeedback to create self-oscillations
 - Change intervals between delay heads
 - Increase/decrease delay length using MAX_DELAY
 - Adjust pot tapers from LINEAR to LOGARITHMIC for taste

## More advanced things to modify/add:

 - Make footswitch 2 ramp cause runaway feedback
 - Make footswitch 2 change the delay time for trippy sounds
 - Add a tone control for the repeats
 - Add analog mojo to the repeats
   - Add 'wow and flutter' by adding imperfections into the repeats
   - Degrade the repeats to mimic an old tape or magnetic drum
 - Add in reverb with a pot for a combo delay/reverb pedal
 - Use the second led to flash shownig the delay time
