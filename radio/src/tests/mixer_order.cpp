/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// Mixes evaluated in channel dependency order (one pass) must give the same
// channel outputs as the index order (up to 5 passes), cycle by cycle, on
// real models driven through sticks, switches and flight mode fades.

#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "gtests.h"
#include "hal/adc_driver.h"
#include "location.h"
#include "storage/yaml/yaml_datastructs.h"
#include "storage/yaml/yaml_parser.h"
#include "storage/yaml/yaml_tree_walker.h"

#if defined(COLORLCD)

extern bool mixerForceIndexOrder;
void loadCurves();
uint8_t mixerOrderCount();

static bool loadModelFile(const std::string& path)
{
  std::ifstream f(path);
  if (!f) return false;
  std::stringstream ss;
  ss << f.rdbuf();
  std::string text = ss.str();

  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);
  YamlParser yp;
  yp.init(YamlTreeWalker::get_parser_calls(), &tree);
  yp.parse(text.c_str(), text.size());
  return true;
}

struct MixerTrace {
  std::vector<int16_t> outputs;  // MAX_OUTPUT_CHANNELS per cycle
  std::vector<uint8_t> flightModes;
  uint8_t orderCount = 0;
};

static MixerTrace runModel(const std::string& path, bool indexOrder,
                           uint32_t seed, int cycles)
{
  MixerTrace trace;
  SYSTEM_RESET();
  MODEL_RESET();
  MIXER_RESET();
  if (!loadModelFile(path)) return trace;
  loadCurves();
  logicalSwitchesInit(true);
  mixerForceIndexOrder = indexOrder;
  g_tmr10ms = 100000;

  std::mt19937 rng(seed);
  const int nAnalogs = adcGetMaxInputs(ADC_INPUT_ALL);
  const int nSwitches = switchGetMaxSwitches();
  std::vector<int> analog(nAnalogs, 0);
  std::uniform_int_distribution<int> step(-300, 300);

  trace.outputs.reserve(cycles * MAX_OUTPUT_CHANNELS);
  for (int k = 0; k < cycles; k++) {
    if (k % 50 == 0) {  // 200 ms: sticks and pots wander
      for (int a = 0; a < nAnalogs; a++) {
        analog[a] = limit(-RESX, analog[a] + step(rng), RESX);
        anaSetFiltered(a, analog[a]);
      }
    }
    if (k % 125 == 0 && nSwitches) {  // 500 ms: a switch moves
      int sw = rng() % nSwitches;
      int pos = (int)(rng() % 3) - 1;
      if (g_model.getSwitchType(sw) != SWITCH_3POS && pos == 0) pos = 1;
      simuSetSwitch(sw, pos);
    }
    // 4 ms mixer period: 10 ms ticks at the real cadence
    uint8_t tick10ms = ((k + 1) * 4) / 10 - (k * 4) / 10;
    g_tmr10ms += tick10ms;
    if (k % 25 == 0) logicalSwitchesTimerTick();  // 100 ms

    evalMixes(tick10ms);

    trace.outputs.insert(trace.outputs.end(), channelOutputs,
                         channelOutputs + MAX_OUTPUT_CHANNELS);
    trace.flightModes.push_back(mixerCurrentFlightMode);
  }
  trace.orderCount = mixerOrderCount();
  mixerForceIndexOrder = false;
  return trace;
}

static void compareModel(const char* name)
{
  std::string path =
      std::string(TESTS_PATH) + "/mixer_models/" + name;
  const int cycles = 30000;  // 2 minutes at 4 ms

  for (uint32_t seed : {1u, 2u, 3u}) {
    MixerTrace ref = runModel(path, true, seed, cycles);
    MixerTrace dep = runModel(path, false, seed, cycles);
    ASSERT_FALSE(ref.outputs.empty()) << "cannot read " << path;
    EXPECT_GT(dep.orderCount, 0) << name << ": dependency order not used";

    std::vector<int> mismatches(MAX_OUTPUT_CHANNELS, 0);
    std::vector<int> maxDiff(MAX_OUTPUT_CHANNELS, 0);
    int firstCycle = -1, firstCh = -1;
    for (int k = 0; k < cycles; k++) {
      for (int ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
        int a = ref.outputs[k * MAX_OUTPUT_CHANNELS + ch];
        int b = dep.outputs[k * MAX_OUTPUT_CHANNELS + ch];
        if (a != b) {
          if (firstCycle < 0) { firstCycle = k; firstCh = ch; }
          mismatches[ch]++;
          maxDiff[ch] = std::max(maxDiff[ch], abs(a - b));
        }
      }
    }
    uint32_t fms = 0;
    for (auto fm : ref.flightModes) fms |= 1 << fm;

    std::ostringstream summary;
    int total = 0;
    for (int ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
      if (!mismatches[ch]) continue;
      total += mismatches[ch];
      summary << " CH" << ch + 1 << ": " << mismatches[ch]
              << " cycles, max diff " << maxDiff[ch] << ";";
    }
    printf("%s seed %u: %d mixes ordered, flight modes 0x%03x, %d mismatches%s\n",
           name, seed, dep.orderCount, fms, total, summary.str().c_str());
    EXPECT_EQ(total, 0) << name << " seed " << seed << ": first mismatch cycle "
                        << firstCycle << " CH" << firstCh + 1;
  }
}

TEST(MixerOrder, esoarplus) { compareModel("esoarplus.yml"); }
TEST(MixerOrder, f3f) { compareModel("f3f.yml"); }
TEST(MixerOrder, dlg6s) { compareModel("dlg6s.yml"); }

#endif
