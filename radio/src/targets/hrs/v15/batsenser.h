#pragma once

#include "definitions.h"

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

void v15BatterySensorInit(void);
/** Re-enable EXT pack/CSD203 sense after EXT +5V tools.
 *  Skips while any EXT tool/Auto session is still active.
 *  Independent of TX main battery (ADC / getBatteryVoltage).
 */
void v15BatterySensorResumeAfterExtPort(void);
void v15BatterySensorTask(void);

uint16_t Getbatsenservol(void);
int16_t Getbatsensercurrent(void);

uint16_t v15BatteryCellVoltage(uint8_t cell_num);
uint16_t v15BatterySystemVoltage(void);
int16_t v15BatterySystemCurrent(void);
uint8_t v15BatteryDetectedCells(void);
bool v15BatteryCell1Present(void);

bool v15BatteryUiTriggerPending(void);
void v15BatteryUiAcknowledgeTrigger(void);

void v15BatteryUiTask(void);

#if defined(__cplusplus)
}
#endif
