#include "radioconfig.h"

AnalogMap getAnalogMapForRadio(const QString& radioName)
{
  const QString name = radioName.toLower();

if (name.contains("tx16smk3") || (name.contains("tx16") && name.contains("mk3")) || name.contains("gx15")) {
    // 0..3: Sticks, 4: S1, 5: S2, 6: LS, 7: RS; switches 0..9: SA..SJ, 10..15: SW1..SW6
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/6, /*rs=*/7, /*pos6=*/255, /*firstFuncSwitch=*/10};
  }
  if (name.contains("tx16") || name.contains("t16") || name.contains("t18") ||
      name.contains("x10") || name.contains("x12s")) {
    // 4: S1, 5: 6POS, 6: S2, 7: LS, 8: RS
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/6, /*ls=*/7, /*rs=*/8, /*pos6=*/5, /*firstFuncSwitch=*/255};
  }

  if (name.contains("boxer")) {
    // 4: S1 (POT1), 5: S2 (POT2), 6: 6POS (POT3)
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/255, /*rs=*/255, /*pos6=*/6, /*firstFuncSwitch=*/255};
  }

if (name.contains("tx15")) {
    // 4: S1, 5: S2, switches 6..11: SW1..SW6
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/255, /*rs=*/255, /*pos6=*/255, /*firstFuncSwitch=*/6};
  }

if (name.contains("gx12")) {
    // 0..3: Sticks, 4: P1 (S1), 5: P2 (S2), 6: SL1 (LS), 7: SL2 (RS); switches 0..7: SA..SH, 8..13: SW1..SW6
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/6, /*rs=*/7, /*pos6=*/255, /*firstFuncSwitch=*/8};
  }

  // TX12, TX12MK2, Zorro, Pocket: 4: S1, 5: S2
  return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/255, /*rs=*/255, /*pos6=*/255, /*firstFuncSwitch=*/255};
}

LcdSpec getLcdSpecForRadio(const QString& radioName)
{
  const QString name = radioName.toLower();
  if (name.contains("tx15")) {
    return {480, 320, 16};
  }

  if (name.contains("nv14") || name.contains("el18") || name.contains("pl18")) {
    return {320, 480, 16};
  }

  if (name.contains("tx16smk3") ||
      (name.contains("tx16") && name.contains("mk3"))) {
    return {800, 480, 16};
  }

  if (name.contains("tx16") || name.contains("t16") || name.contains("t18") ||
      name.contains("t15") || name.contains("x10") || name.contains("x12s") ||
      name.contains("gx15")) {
    return {480, 272, 16};
  }

  return {128, 64, 1};
}