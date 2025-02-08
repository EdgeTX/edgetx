#pragma once

#include "dataconstants.h"

#if defined(VCONTROLS) && defined(COLORLCD)
extern int16_t virtualInputs[MAX_VIRTUAL_INPUTS];
extern uint64_t virtualSwitches; 
#endif
