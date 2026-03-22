/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "easymode_convert.h"

#include "edgetx.h"
#include "mixes.h"
#include "input_mapping.h"
#include "hal/adc_driver.h"

#include <string.h>

// Helper: add a mix entry, returns next free mix index
static uint8_t addMix(uint8_t idx, uint8_t destCh, uint16_t srcRaw,
                       int16_t weight, uint8_t mltpx = 0 /* ADD */)
{
  if (idx >= MAX_MIXERS) return idx;
  MixData* mix = mixAddress(idx);
  memset(mix, 0, sizeof(MixData));
  mix->destCh = destCh;
  mix->srcRaw = srcRaw;
  mix->weight = makeSourceNumVal(weight);
  mix->mltpx = mltpx;
  return idx + 1;
}

// Helper: add an input/expo entry, returns next free expo index
static uint8_t addInput(uint8_t idx, uint8_t chn, uint16_t srcRaw,
                         int16_t weight, int8_t expo, const char* name)
{
  if (idx >= MAX_EXPOS) return idx;
  ExpoData* ed = expoAddress(idx);
  memset(ed, 0, sizeof(ExpoData));
  ed->chn = chn;
  ed->srcRaw = srcRaw;
  ed->weight = weight;
  ed->mode = 3;  // both directions
  if (expo != 0) {
    ed->curve.type = CURVE_REF_EXPO;
    ed->curve.value = expo;
  }
  if (name) {
    strncpy(g_model.inputNames[chn], name, LEN_INPUT_NAME);
  }
  return idx + 1;
}

// Helper: name an output channel
static void nameOutput(uint8_t ch, const char* name)
{
  if (ch < MAX_OUTPUT_CHANNELS && name) {
    strncpy(g_model.limitData[ch].name, name, LEN_CHANNEL_NAME);
  }
}

// ---- Airplane Mixes ----
static void generateAirplaneMixes(const EasyModeData& em,
                                   uint8_t& mixIdx, uint8_t& expoIdx)
{
  auto& ch = em.channels;

  // Input 0: Aileron (with expo)
  if (ch.aileron >= 0) {
    expoIdx = addInput(expoIdx, 0, MIXSRC_FIRST_STICK + inputMappingChannelOrder(0),
                        100, em.options.expoAileron, "Ail");
    // Mix: Input 0 -> aileron channel
    mixIdx = addMix(mixIdx, ch.aileron, 1, 100);  // srcRaw=1 means Input 0
    nameOutput(ch.aileron, "Aileron");
  }

  // Input 1: Elevator (with expo)
  if (ch.elevator >= 0) {
    expoIdx = addInput(expoIdx, 1, MIXSRC_FIRST_STICK + inputMappingChannelOrder(1),
                        100, em.options.expoElevator, "Ele");
    mixIdx = addMix(mixIdx, ch.elevator, 2, 100);
    nameOutput(ch.elevator, "Elevator");
  }

  // Input 2: Throttle (or motor switch for glider with motor)
  if (ch.throttle >= 0 && em.motorType != EASYMOTOR_NONE) {
    if (em.modelType == EASYMODE_GLIDER && em.sources.motorSource != MIXSRC_NONE) {
      // Glider with motor: motor controlled by switch, throttle stick free for crow
      expoIdx = addInput(expoIdx, 2, em.sources.motorSource, 100, 0, "Motor");
      nameOutput(ch.throttle, "Motor");
    } else {
      expoIdx = addInput(expoIdx, 2, MIXSRC_FIRST_STICK + inputMappingChannelOrder(2),
                          100, 0, "Thr");
      nameOutput(ch.throttle, "Throttle");
    }
    mixIdx = addMix(mixIdx, ch.throttle, 3, 100);
  }

  // Input 3: Rudder (with expo)
  if (ch.rudder >= 0) {
    expoIdx = addInput(expoIdx, 3, MIXSRC_FIRST_STICK + inputMappingChannelOrder(3),
                        100, em.options.expoRudder, "Rud");
    mixIdx = addMix(mixIdx, ch.rudder, 4, 100);
    nameOutput(ch.rudder, "Rudder");
  }

  // Dual aileron: second aileron channel (inverted for roll)
  if (ch.aileron2 >= 0 &&
      (em.wingType == EASYWING_DUAL_AIL || em.wingType == EASYWING_FLAPERON ||
       em.wingType == EASYWING_2AIL_1FLAP ||
       em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP)) {
    int16_t weight = -(100 - em.options.aileronDifferential);
    mixIdx = addMix(mixIdx, ch.aileron2, 1, weight);  // Input 0 inverted
    nameOutput(ch.aileron2, "Ail R");
    nameOutput(ch.aileron, "Ail L");
  }

  // ---- Flap Input and Mixes ----
  bool needsFlapInput = (em.wingType == EASYWING_FLAPERON ||
                         em.wingType == EASYWING_1AIL_1FLAP ||
                         em.wingType == EASYWING_2AIL_1FLAP ||
                         em.wingType == EASYWING_2AIL_2FLAP ||
                         em.wingType == EASYWING_2AIL_4FLAP);

  if (needsFlapInput && em.sources.flapSource != MIXSRC_NONE) {
    expoIdx = addInput(expoIdx, 4, em.sources.flapSource, 100, 0, "Flap");
    uint8_t flapMixSrc = 5;  // Input 4 = srcRaw 5

    // Flaperon: flap input droops both ailerons together
    if (em.wingType == EASYWING_FLAPERON && ch.aileron >= 0 && ch.aileron2 >= 0) {
      mixIdx = addMix(mixIdx, ch.aileron, flapMixSrc, 50);
      mixIdx = addMix(mixIdx, ch.aileron2, flapMixSrc, 50);
      nameOutput(ch.aileron, "FlpAil L");
      nameOutput(ch.aileron2, "FlpAil R");
    }

    // Single flap channel
    if (ch.flap >= 0 &&
        (em.wingType == EASYWING_1AIL_1FLAP || em.wingType == EASYWING_2AIL_1FLAP)) {
      mixIdx = addMix(mixIdx, ch.flap, flapMixSrc, 100);
      nameOutput(ch.flap, "Flap");
    }

    // Dual flap channels
    if (ch.flap >= 0 && ch.flap2 >= 0 &&
        (em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP)) {
      mixIdx = addMix(mixIdx, ch.flap, flapMixSrc, 100);
      mixIdx = addMix(mixIdx, ch.flap2, flapMixSrc, 100);
      nameOutput(ch.flap, "Flap L");
      nameOutput(ch.flap2, "Flap R");
    }

    // Full house: brake/inner flap channels
    if (em.wingType == EASYWING_2AIL_4FLAP) {
      if (ch.flap3 >= 0) {
        mixIdx = addMix(mixIdx, ch.flap3, flapMixSrc, 100);
        nameOutput(ch.flap3, "Brake L");
      }
      if (ch.flap4 >= 0) {
        mixIdx = addMix(mixIdx, ch.flap4, flapMixSrc, 100);
        nameOutput(ch.flap4, "Brake R");
      }
    }

    // Flap-to-elevator compensation
    if (em.options.flapToElevatorComp != 0 && ch.elevator >= 0) {
      mixIdx = addMix(mixIdx, ch.elevator, flapMixSrc,
                       em.options.flapToElevatorComp);
    }
  }

  // Elevon mixing (flying wing / delta)
  if (em.wingType == EASYWING_ELEVON || em.wingType == EASYWING_DELTA) {
    if (ch.aileron >= 0 && ch.aileron2 >= 0) {
      // Left elevon = Aileron + Elevator
      // Right elevon = -Aileron + Elevator
      // Clear previous mixes for these channels and re-do
      // We use Input 0 (Ail) and Input 1 (Ele)
      // Note: we already set up Input 0 and Input 1 above
      // Re-assign the mixes for elevon

      // Find and overwrite: Left elevon
      MixData* m;
      // Left channel: Ail + Ele
      m = mixAddress(0);
      memset(m, 0, sizeof(MixData));
      m->destCh = ch.aileron;
      m->srcRaw = 1;  // Input 0 (Ail)
      m->weight = makeSourceNumVal(50);
      m->mltpx = 0;  // ADD

      m = mixAddress(1);
      memset(m, 0, sizeof(MixData));
      m->destCh = ch.aileron;
      m->srcRaw = 2;  // Input 1 (Ele)
      m->weight = makeSourceNumVal(50);
      m->mltpx = 0;  // ADD

      // Right channel: -Ail + Ele
      m = mixAddress(2);
      memset(m, 0, sizeof(MixData));
      m->destCh = ch.aileron2;
      m->srcRaw = 1;  // Input 0 (Ail)
      m->weight = makeSourceNumVal(-50);
      m->mltpx = 0;  // ADD

      m = mixAddress(3);
      memset(m, 0, sizeof(MixData));
      m->destCh = ch.aileron2;
      m->srcRaw = 2;  // Input 1 (Ele)
      m->weight = makeSourceNumVal(50);
      m->mltpx = 0;  // ADD

      mixIdx = 4;

      // Add throttle and rudder after elevon mixes
      if (ch.throttle >= 0 && em.motorType != EASYMOTOR_NONE) {
        mixIdx = addMix(mixIdx, ch.throttle, 3, 100);
      }
      if (ch.rudder >= 0) {
        mixIdx = addMix(mixIdx, ch.rudder, 4, 100);
      }

      nameOutput(ch.aileron, "L Elevon");
      nameOutput(ch.aileron2, "R Elevon");
    }
  }

  // V-tail mixing
  if (em.tailType == EASYTAIL_V_TAIL && ch.elevator >= 0 && ch.rudder >= 0) {
    // Left ruddervator = Elevator + Rudder
    // Right ruddervator = Elevator - Rudder
    // Override elevator and rudder mixes

    // Find the elevator mix and make it elevon-style
    for (uint8_t i = 0; i < mixIdx; i++) {
      MixData* m = mixAddress(i);
      if (m->destCh == ch.elevator && m->srcRaw == 2) {
        // Elevator channel: Ele + Rud
        m->weight = makeSourceNumVal(50);  // reduce elevator weight
        // Add rudder to same channel
        mixIdx = addMix(mixIdx, ch.elevator, 4, 50);
        break;
      }
    }
    for (uint8_t i = 0; i < mixIdx; i++) {
      MixData* m = mixAddress(i);
      if (m->destCh == ch.rudder && m->srcRaw == 4) {
        // Rudder channel: Ele - Rud
        m->srcRaw = 2;   // Elevator input
        m->weight = makeSourceNumVal(50);
        // Add inverted rudder
        mixIdx = addMix(mixIdx, ch.rudder, 4, -50);
        break;
      }
    }
    nameOutput(ch.elevator, "V-Tail L");
    nameOutput(ch.rudder, "V-Tail R");
  }

  // Dual elevator: both halves move together for pitch
  if (em.tailType == EASYTAIL_DUAL_ELEVATOR && ch.elevator >= 0 && ch.elevator2 >= 0) {
    mixIdx = addMix(mixIdx, ch.elevator2, 2, 100);  // Input 1 (Ele) same direction
    nameOutput(ch.elevator, "Ele L");
    nameOutput(ch.elevator2, "Ele R");
  }

  // Ailevator: elevator halves also have roll authority
  if (em.tailType == EASYTAIL_AILEVATOR && ch.elevator >= 0 && ch.elevator2 >= 0) {
    // Reduce existing elevator mix to 50% to leave room for aileron component
    for (uint8_t i = 0; i < mixIdx; i++) {
      MixData* m = mixAddress(i);
      if (m->destCh == ch.elevator && m->srcRaw == 2) {
        m->weight = makeSourceNumVal(50);
        break;
      }
    }
    // Left elevator: Ele(50%) + Ail(50%)
    mixIdx = addMix(mixIdx, ch.elevator, 1, 50);
    // Right elevator: Ele(50%) - Ail(50%)
    mixIdx = addMix(mixIdx, ch.elevator2, 2, 50);
    mixIdx = addMix(mixIdx, ch.elevator2, 1, -50);
    nameOutput(ch.elevator, "Ailevtr L");
    nameOutput(ch.elevator2, "Ailevtr R");
  }

  // Aileron to rudder mix
  if (em.options.aileronToRudderMix > 0 && ch.rudder >= 0) {
    // Add aileron input mixed into rudder channel
    mixIdx = addMix(mixIdx, ch.rudder, 1,
                     em.options.aileronToRudderMix);
  }

  // Crow braking: flap input also drives ailerons into reflex (trailing edge up)
  // This creates drag for steep approach without gaining speed
  if (em.options.crowEnabled) {
    uint8_t flapMixSrc = 5;  // Input 4 (Flap)

    // Aileron reflex: both ailerons go trailing edge up (opposite to flap)
    if (ch.aileron >= 0) {
      mixIdx = addMix(mixIdx, ch.aileron, flapMixSrc, -30);
    }
    if (ch.aileron2 >= 0) {
      mixIdx = addMix(mixIdx, ch.aileron2, flapMixSrc, -30);
    }
  }
}

// ---- Helicopter Mixes ----
static void generateHelicopterMixes(const EasyModeData& em,
                                     uint8_t& mixIdx, uint8_t& expoIdx)
{
  auto& ch = em.channels;

  // Set up swash ring data
  g_model.swashR.type = em.options.swashType;
  g_model.swashR.value = 60;  // default swash ring value
  g_model.swashR.collectiveSource = MIXSRC_FIRST_STICK + inputMappingChannelOrder(2);
  g_model.swashR.aileronSource = MIXSRC_FIRST_STICK + inputMappingChannelOrder(0);
  g_model.swashR.elevatorSource = MIXSRC_FIRST_STICK + inputMappingChannelOrder(1);
  g_model.swashR.collectiveWeight = 100;
  g_model.swashR.aileronWeight = 100;
  g_model.swashR.elevatorWeight = 100;

  // Standard 4-channel setup
  // Input 0: Aileron (cyclic)
  expoIdx = addInput(expoIdx, 0, MIXSRC_FIRST_STICK + inputMappingChannelOrder(0),
                      100, em.options.expoAileron, "Ail");
  // Input 1: Elevator (cyclic)
  expoIdx = addInput(expoIdx, 1, MIXSRC_FIRST_STICK + inputMappingChannelOrder(1),
                      100, em.options.expoElevator, "Ele");
  // Input 2: Throttle / Collective
  expoIdx = addInput(expoIdx, 2, MIXSRC_FIRST_STICK + inputMappingChannelOrder(2),
                      100, 0, "Thr");
  // Input 3: Rudder (tail rotor)
  expoIdx = addInput(expoIdx, 3, MIXSRC_FIRST_STICK + inputMappingChannelOrder(3),
                      100, em.options.expoRudder, "Rud");

  // Channel mixes (1:1 for now, swashR handles CCPM internally)
  if (ch.aileron >= 0)
    mixIdx = addMix(mixIdx, ch.aileron, 1, 100);
  if (ch.elevator >= 0)
    mixIdx = addMix(mixIdx, ch.elevator, 2, 100);
  if (ch.throttle >= 0)
    mixIdx = addMix(mixIdx, ch.throttle, 3, 100);
  if (ch.rudder >= 0)
    mixIdx = addMix(mixIdx, ch.rudder, 4, 100);

  nameOutput(ch.aileron, "Aileron");
  nameOutput(ch.elevator, "Elevator");
  nameOutput(ch.throttle, "Throttle");
  nameOutput(ch.rudder, "Rudder");

  // Gyro gain channel: direct source pass-through
  if (ch.aux1 >= 0 && em.sources.aux1Source != MIXSRC_NONE) {
    expoIdx = addInput(expoIdx, 4, em.sources.aux1Source, 100, 0, "Gyro");
    mixIdx = addMix(mixIdx, ch.aux1, 5, 100);  // Input 4 = srcRaw 5
    nameOutput(ch.aux1, "GyroGain");
  }
}

// ---- Glider Mixes ----
static void generateGliderMixes(const EasyModeData& em,
                                  uint8_t& mixIdx, uint8_t& expoIdx)
{
  generateAirplaneMixes(em, mixIdx, expoIdx);
}

// ---- Multirotor Mixes ----
static void generateMultirotorMixes(const EasyModeData& em,
                                     uint8_t& mixIdx, uint8_t& expoIdx)
{
  auto& ch = em.channels;

  // For multirotors, mixes are simple 1:1 passthrough.
  // The flight controller handles all mixing.
  // Channel order depends on FC protocol (AETR, TAER, etc.)

  // Input 0-3: sticks mapped per channel order
  uint8_t order[4];
  switch (em.options.multiChannelOrder) {
    case EASYMULTI_TAER:
      order[0] = 2; order[1] = 0; order[2] = 1; order[3] = 3; // T A E R
      break;
    case EASYMULTI_RETA:
      order[0] = 3; order[1] = 1; order[2] = 2; order[3] = 0; // R E T A
      break;
    case EASYMULTI_AETR:
    default:
      order[0] = 0; order[1] = 1; order[2] = 2; order[3] = 3; // A E T R
      break;
  }

  const char* names[] = {"Ail", "Ele", "Thr", "Rud"};
  const char* outNames[] = {"Roll", "Pitch", "Throttle", "Yaw"};

  for (int i = 0; i < 4; i++) {
    uint8_t stickIdx = order[i];
    expoIdx = addInput(expoIdx, i,
                        MIXSRC_FIRST_STICK + inputMappingChannelOrder(stickIdx),
                        100,
                        (stickIdx == 2) ? 0 : em.options.expoAileron,  // no expo on throttle
                        names[stickIdx]);
    mixIdx = addMix(mixIdx, i, i + 1, 100);  // Input i -> CH(i+1)
    nameOutput(i, outNames[stickIdx]);
  }

  // Arm switch on aux1 (CH5): direct source pass-through
  if (ch.aux1 >= 0 && em.sources.aux1Source != MIXSRC_NONE) {
    uint8_t auxChn = 4;  // Input 4
    expoIdx = addInput(expoIdx, auxChn, em.sources.aux1Source, 100, 0, "Arm");
    mixIdx = addMix(mixIdx, ch.aux1, auxChn + 1, 100);
    nameOutput(ch.aux1, "Arm");
  }

  // Flight mode switch on aux2 (CH6): direct source pass-through
  if (ch.aux2 >= 0 && em.sources.aux2Source != MIXSRC_NONE) {
    uint8_t auxChn = 5;  // Input 5
    expoIdx = addInput(expoIdx, auxChn, em.sources.aux2Source, 100, 0, "Mode");
    mixIdx = addMix(mixIdx, ch.aux2, auxChn + 1, 100);
    nameOutput(ch.aux2, "Mode");
  }
}

// ---- Surface Vehicle Mixes ----
static void generateSurfaceMixes(const EasyModeData& em,
                                   uint8_t& mixIdx, uint8_t& expoIdx)
{
  auto& ch = em.channels;

  // Input 0: Steering
  if (ch.steering >= 0) {
    expoIdx = addInput(expoIdx, 0, MIXSRC_FIRST_STICK + inputMappingChannelOrder(0),
                        100, em.options.expoAileron, "Steer");
    mixIdx = addMix(mixIdx, ch.steering, 1, 100);
    nameOutput(ch.steering, "Steering");
  }

  // Input 1: Throttle
  if (ch.throttle >= 0) {
    expoIdx = addInput(expoIdx, 1, MIXSRC_FIRST_STICK + inputMappingChannelOrder(2),
                        100, 0, "Thr");
    mixIdx = addMix(mixIdx, ch.throttle, 2, 100);
    nameOutput(ch.throttle, "Throttle");
  }
}

// ---- Main entry point ----
void easyModeApply(const EasyModeData& em)
{
  if (em.modelType == EASYMODE_NONE) return;

  // Clear existing mixes and inputs
  memset(g_model.mixData, 0, sizeof(g_model.mixData));
  memset(g_model.expoData, 0, sizeof(g_model.expoData));
  memset(g_model.inputNames, 0, sizeof(g_model.inputNames));

  // Reset swash ring data
  memset(&g_model.swashR, 0, sizeof(g_model.swashR));

  uint8_t mixIdx = 0;
  uint8_t expoIdx = 0;

  switch (em.modelType) {
    case EASYMODE_AIRPLANE:
      generateAirplaneMixes(em, mixIdx, expoIdx);
      break;

    case EASYMODE_HELICOPTER:
      generateHelicopterMixes(em, mixIdx, expoIdx);
      break;

    case EASYMODE_GLIDER:
      generateGliderMixes(em, mixIdx, expoIdx);
      break;

    case EASYMODE_MULTIROTOR:
      generateMultirotorMixes(em, mixIdx, expoIdx);
      break;

    case EASYMODE_CAR:
    case EASYMODE_BOAT:
      generateSurfaceMixes(em, mixIdx, expoIdx);
      break;

    default:
      break;
  }

  // Append custom free-form mixes
  for (int i = 0; i < EASY_CUSTOM_MIXES; i++) {
    auto& cm = em.customMix[i];
    if (cm.source != MIXSRC_NONE && cm.destCh >= 0 && cm.weight != 0) {
      mixIdx = addMix(mixIdx, cm.destCh, cm.source, cm.weight);
    }
  }
}

void easyModeConvertToExpert()
{
  // Just clear the easy mode data — the mixes are already in g_model
  easyModeClear();
  storageDirty(EE_MODEL);
}
