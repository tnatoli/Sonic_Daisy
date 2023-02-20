// PedalPCB Terrarium Header
// Copyright (C) 2020 PedalPCB.com
// http://www.pedalpcb.com

namespace terrarium
{
    class Terrarium
    {
    public:
	enum Sw
	    {
	     FOOTSWITCH_1 = 4,
	     FOOTSWITCH_2 = 5,
	     SWITCH_1 = 2,
	     SWITCH_2 = 1,
	     SWITCH_3 = 0,
	     SWITCH_4 = 6
	    };
	
	enum Knob
	    {
	     KNOB_1 = 0,
	     KNOB_2 = 2,
	     KNOB_3 = 4,
	     KNOB_4 = 1,
	     KNOB_5 = 3,
	     KNOB_6 = 5
	    };
	enum LED
	    {
	     LED_1 = 22,
	     LED_2 = 23
	    };
    };
}
