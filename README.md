# Sonic_Daisy
This is the public repository for code for the [Daisy Embedded Audio Platform](https://www.electro-smith.com/daisy) written by Sonic Explorer. 

This repository is dependent on modules included in the [DaisyExamples repository from Electro-Smith.](https://github.com/electro-smith/DaisyExamples)
If this repository is placed the same directory that houses the DaisyExamples repository the Makefiles should work as they are.
If you place this respository somewhere else you will have to edit the paths in the Makefiles. 

# Included Programs

## rhythm_delay_basic 
This is a program meant to be used with the [Terrarium PCB by PedalPCB](https://www.pedalpcb.com/product/pcb351/). It is a rhythmic delay made up of four separate delays with the same fixed spacings as the Binson Echorec (sixteenth note, eighth note, dotted eighth note, quarter note), but with the ability to adjuste the overall delay time. Controls are overall delay time, wet/dry mix, and feedback. Each delay can be individually turned on/off via switches.
