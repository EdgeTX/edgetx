#include "vcontrols.h"

#include "edgetx.h"

#if defined(VCONTROLS) && defined(COLORLCD)
int16_t virtualInputs[MAX_VIRTUAL_INPUTS] = {};
uint64_t virtualSwitches = 0; 
#endif
