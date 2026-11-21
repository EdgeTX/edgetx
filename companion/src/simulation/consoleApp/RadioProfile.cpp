/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "RadioProfile.h"

namespace SimulatorCli
{

AnalogMap getAnalogMapForRadio(const QString& radioName)
{
  const QString name = radioName.toLower();

  if (name.contains("tx16smk3") ||
      (name.contains("tx16") && name.contains("mk3")) ||
      name.contains("gx15")) {
    // 0..3: Sticks, 4: S1, 5: S2, 6: LS, 7: RS;
    // switches 0..9: SA..SJ, 10..15: SW1..SW6
    return {0,
            1,
            2,
            3,
            /*s1=*/4,
            /*s2=*/5,
            /*ls=*/6,
            /*rs=*/7,
            /*pos6=*/INVALID_AXIS,
            /*firstFuncSwitch=*/10};
  }

  if (name.contains("tx16") || name.contains("t16") || name.contains("t18") ||
      name.contains("x10") || name.contains("x12s")) {
    // 4: S1, 5: 6POS, 6: S2, 7: LS, 8: RS
    return {0,
            1,
            2,
            3,
            /*s1=*/4,
            /*s2=*/6,
            /*ls=*/7,
            /*rs=*/8,
            /*pos6=*/5,
            /*firstFuncSwitch=*/INVALID_AXIS};
  }

  if (name.contains("boxer")) {
    // 4: S1 (POT1), 5: S2 (POT2), 6: 6POS (POT3)
    return {0,
            1,
            2,
            3,
            /*s1=*/4,
            /*s2=*/5,
            /*ls=*/INVALID_AXIS,
            /*rs=*/INVALID_AXIS,
            /*pos6=*/6,
            /*firstFuncSwitch=*/INVALID_AXIS};
  }

  if (name.contains("tx15")) {
    // 4: S1, 5: S2, switches 6..11: SW1..SW6
    return {0,
            1,
            2,
            3,
            /*s1=*/4,
            /*s2=*/5,
            /*ls=*/INVALID_AXIS,
            /*rs=*/INVALID_AXIS,
            /*pos6=*/INVALID_AXIS,
            /*firstFuncSwitch=*/6};
  }

  if (name.contains("gx12")) {
    // 0..3: Sticks, 4: P1 (S1), 5: P2 (S2), 6: SL1 (LS), 7: SL2 (RS);
    // switches 0..7: SA..SH, 8..13: SW1..SW6
    return {0,
            1,
            2,
            3,
            /*s1=*/4,
            /*s2=*/5,
            /*ls=*/6,
            /*rs=*/7,
            /*pos6=*/INVALID_AXIS,
            /*firstFuncSwitch=*/8};
  }

  // TX12, TX12MK2, Zorro, Pocket: 4: S1, 5: S2
  return {0,
          1,
          2,
          3,
          /*s1=*/4,
          /*s2=*/5,
          /*ls=*/INVALID_AXIS,
          /*rs=*/INVALID_AXIS,
          /*pos6=*/INVALID_AXIS,
          /*firstFuncSwitch=*/INVALID_AXIS};
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

RadioProfile getRadioProfile(const QString& radioName)
{
  const QString name = radioName.toLower();

  RadioProfile profile;
  profile.analogs = getAnalogMapForRadio(radioName);
  profile.lcd = getLcdSpecForRadio(radioName);

  if (name.contains("tx16smk3") ||
      (name.contains("tx16") && name.contains("mk3"))) {
    profile.numSwitches = 10;  // SA..SJ
  } else {
    profile.numSwitches = 8;  // SA..SH
  }

  return profile;
}

}  // namespace SimulatorCli