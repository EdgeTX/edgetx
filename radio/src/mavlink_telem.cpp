/*
 * (c) www.olliw.eu, OlliW, OlliW42
 */

#include "opentx.h"

MAVLINK_RAM_SECTION MavlinkTelem mavlinkTelem;

// -- Wakeup call from OpenTx --
// this is the main entry point

// ourself = link 0
// serial1 = link 1
// serial2 = link 2
// usb     = link 3

void MavlinkTelem::wakeup()
{
}

// -- 10 ms tick --

void MavlinkTelem::tick10ms()
{
}

// -- Resets --

void MavlinkTelem::_init(void)
{
}
