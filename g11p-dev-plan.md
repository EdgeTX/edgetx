# G11P Target Development Plan

Add a new **FlySky G11P** target to EdgeTX, based on the existing **ST16** target.

The relationship is the same as **NB4P : PL18** — same hardware platform, very similar
pinout, air radio vs surface radio. Therefore the target is implemented as a revision of
the ST16 platform: **`PCB=ST16`, `PCBREV=G11P`, flavour `g11p`** (exactly how NB4P is a
`PCBREV` of PL18).

- MCU: **STM32H750IBT6**, LQFP176 (identical to ST16).
- External NOR: QSPI (`W25Q64CVSS`), same QSPI bank/pins as ST16.
- Companion mapping file: [`st16-vs-g11p-pinout.md`](st16-vs-g11p-pinout.md).

---

## 1. Reference model (PL18 / NB4P revision pattern)

`radio/src/targets/pl18/CMakeLists.txt` branches on `PCBREV` (PL18U / EL18 / NV14 / NB4P /
PL18EV). Each branch sets:

- `FLAVOUR`
- a `RADIO_*` compile definition
- the key driver (`KEY_DRIVER`, e.g. `nb4p_key_driver.cpp`)
- module list (`INTERNAL_MODULES`, `DEFAULT_INTERNAL_MODULE`)

NB4P is the surface variant: analog `TH`/`ST` sticks, ADC-ladder switches/trims/keys
(`nb4p_key_driver.cpp`, `nb4p_switch_driver.cpp`), no PCA95xx I/O expander.

G11P follows the same pattern, but inside `radio/src/targets/st16/`, guarded by
`RADIO_G11P`.

---

## 2. Deliverable A — `st16-vs-g11p-pinout.md` (project root)

A pin-by-pin comparison of the two boards' STM32H750 LQFP176, mirroring the column format
of `~/PL20-PL18U.xlsx`: `# | Pin Name | Pin # | Net Label (ST16) | Net Label (G11P) | Diff?`.

### Sources and method

- **ST16 net labels**: `FS-ST16-Main V1.5 20240426.pdf`, **sheet 4 of 11**, the
  `STM32H750IBT6_LQFP176-24X24MM` MCU sheet. (Sheet 5 is the STM32F072C8T6 RF-module MCU;
  sheet 10 is the PCA9555/NCA9555 I/O expander; sheet 11 is the revision changelog.)
- **G11P net labels**: `FS-G11P-V1.0原理图及点位图-20250109.pdf`, MCU sheet (page 2,
  `STM32H750IBT6_LQFP176`).
- **Pin name / pin number**: paired directly from each schematic symbol's edges with
  `pdftotext -bbox` + coordinate clustering (top 133-176, left 1-44, bottom 45-88,
  right 89-132). The resulting LQFP176 pinout was independently cross-checked against
  `radio/src/targets/st16/{hal.h,board.h,bsp_io.h,sdram_driver.cpp,lcd_driver.cpp,extflash_driver.cpp}`.

### Result: the boards are almost pin-identical

Of the 176 package pins, only **34 I/O nets differ**; every other functional pin
(all FMC/SDRAM, QSPI, LTDC RGB + sync, SDIO, audio SPI1, backlight, rotary encoder,
power button, USB, SWD, RF-module UART, etc.) is net-identical.

Delta (from the generated mapping file):

| Pin | Pin # | ST16 | G11P |
|---|---:|---|---|
| PE5 | 4 | PPM_IN | RF_PWR_ON |
| PE6 | 5 | PPM_OUT | TP_INT |
| PC13 | 8 | DFU_DET | DFU |
| PI9 | 11 | UART4_RX_HEAD | TP_RESET |
| PI11 | 13 | BACK_KEY4 | LCD_NRST |
| PF6 | 24 | SWF | PPM_IN |
| PF7 | 25 | SWD | PPM_OUT |
| PC0 | 32 | VBUS_ADC | 2S_ADC |
| PC2 | 34 | SWA | VR2A |
| PC3 | 35 | SWB | SW4_ADC |
| PA0 | 40 | CH1 | TH_ADC |
| PA1 | 41 | CH2 | ST_ADC |
| PA2 | 42 | CH3 | TR3_ADC |
| PH4 | 45 | BAT2_ADC | SW1_ADC |
| PA3 | 47 | CH4 | TR4_ADC |
| PA6 | 52 | VRA | SW2_ADC |
| PA7 | 53 | VRC | SW3_ADC |
| PC5 | 55 | KEY_EXIT | TR_ADC |
| PB0 | 56 | VRD | K12_ADC |
| PB1 | 57 | VRB | K34_ADC |
| PH6 | 83 | PCA9555PW_NINT | USB_SW |
| PH7 | 84 | I2C3_SCL | CHARGE_ENB |
| PH8 | 85 | I2C3_SDA | LCD_CS |
| PB12 | 92 | UART5_RX_SPORT | RF_DFU_C |
| PB13 | 93 | UART5_TX_SPORT | LED_DATA2 |
| PC7 | 116 | GEMO_NINT | K6A |
| PA8 | 119 | KEY_MENU | K7A |
| PI3 | 134 | BACK_KEY3 | VBUS_I |
| PA15 | 138 | SHINE_LAMP_PWM | LED_DATA_IN |
| PD5 | 147 | HALL_RX | PA_NMUTE |
| PD6 | 150 | HALL_TX | VS1053B_RST |
| PB7 | 165 | SENSOR_SDA | HW_VER |
| PB8 | 167 | SENSOR_SCL | TP_SCL |
| PB9 | 168 | UART4_TX_PPM | TP_SDA |

Notes / implications:

- **No I/O expander on G11P.** ST16 uses a PCA9555/NCA9555 on I2C3 (PH7/PH8, INT PH6);
  G11P repurposes PH6/PH7/PH8 and drives inputs from the ADC ladders + direct GPIO.
- **Inputs are surface-radio style (NB4P-like)**: `TH_ADC`/`ST_ADC` sticks (pure analog),
  `SW1-4_ADC` + `VR2A`, `K12_ADC`/`K34_ADC` keys, `TR1-4_ADC` trims.
- **LCD is 320x480 portrait**, not 480x320: the P4 note reads `320*480*32/8=614.4KB` and
  the panel is `MS0621-32TFL11E` (same orientation model as NB4P).
- **LEDs**: a single WS2812 chain of 2 logical LEDs (`LED_STRIP_LENGTH = 2`); the 4 physical
  `HI-1204RGBC` parts are two parallel pairs. `LED_DATA_IN` (PA15) drives the chain;
  `LED_DATA2` (PB13) role to confirm.
- **PPM moved** PE5/PE6 -> PF6/PF7. **No external module bay**, but the trainer port
  (`PPM_IN`/`PPM_OUT` = PF6/PF7 = **UART7**) is exposed as the **external module UART**.
  PE5/PE6 become RF power-on / touch interrupt.
- **SPORT UART5 (PB12/PB13)** is gone; PB12/PB13 are RF DFU / LED data.
- **Touch** moves to PB9 (SDA) / PB8 (SCL) / PE6 (INT) / PI9 (reset); reuse the PL18
  standard FlySky touch driver. The old ST16 `SENSOR_*` (IMU) I2C lines on PB7/PB8 are not
  present.
- **Absent on G11P** (per schematic scan): external module bay, Bluetooth, wireless charger,
  S.Port, IMU/gyro - so those ST16 features are dropped.
- **Flash note (cross-validation finding)**: ST16 `hal.h` defines
  `FLASH_SPI = SPI6` on PG6/PG12/PG13/PG14, but the ST16 schematic routes the NOR as
  `FLASH_SPI2` on **PB4/PA9/PB14/PB15**, and `extflash_driver.cpp` actually drives the
  **QSPI** NOR (`stm32_qspi_nor_*`). Those `FLASH_SPI`/SPI6 macros are vestigial. G11P uses
  the same `FLASH_SPI2` pins and the same QSPI-flash approach.

---

## 3. Deliverable B — radio firmware target

### 3.1 Registration

- `radio/src/CMakeLists.txt`: `ST16` is already in `PCB_TYPES`; keep the
  `elseif(PCB STREQUAL ST16)` include unchanged.
- `radio/src/targets/st16/CMakeLists.txt`: refactor to branch on `PCBREV`, like PL18:
  - `if(PCBREV STREQUAL G11P)`
    - `set(FLAVOUR g11p)`
    - `add_definitions(-DRADIO_G11P)`
    - select `g11p_key_driver.cpp` / `g11p_switch_driver.cpp`
    - keep `AFHDS3`, `LED_STRIP`, `USE_VS1053B` as applicable
  - `else()` existing ST16 behaviour (`-DRADIO_ST16`).
  - Keep the common `-DPCBST16 -DPCBHORUS -DPCBFLYSKY`.
- `tools/build-common.sh`: add
  `g11p) BUILD_OPTIONS+="-DPCB=ST16 -DPCBREV=G11P"` (ST16 case unchanged).
- `tools/build-flysky.py`: add `"G11P": { "PCB": "ST16", "PCBREV": "G11P", "NANO": "NO" }`.
- `tools/generate-hw-defs.sh`, `tools/generate-yaml.sh`, `tools/build-companion.sh`,
  `.github/workflows/actions.yml`: add `g11p` (and to the build matrix / commit-tests).
- `fw.json`: add `["Flysky G11P", "g11p-"]`.

### 3.2 HAL / board

- `radio/src/targets/st16/hal.h`: wrap G11P-specific definitions in
  `#if defined(RADIO_G11P)` (module power/RF, touch, audio RST/MUTE, LCD CS/NRST/ID/SDA/SCL,
  SPI NOR bank, ADC key/switch/trim channels, battery, PPM, external module UART, LED,
  haptic/motor). Set `LCD_W=320`, `LCD_H=480` (portrait).
- **LCD / LTDC**: keep the same RGB pin mapping but add 320x480 timings/polarity in
  `lcd_driver.cpp` for G11P (exact porch/polarity from the `MS0621-32TFL11E` datasheet -
  TODO).
- **Touch**: reuse `targets/pl18/touch_driver.cpp` (standard FlySky touch) rather than
  ST16's `tp_cst340`, wiring `TOUCH_*` to PB9/PB8 (I2C) + PE6 (INT) + PI9 (RST).
- **External module**: **no bay**, but expose the trainer port as the ext-module UART -
  `EXTMODULE` with `EXTMODULE_USART = UART7`, TX=PF7 / RX=PF6. This replaces ST16's UART4
  ext module and its PE5/PE6 TIM15 trainer.
- `radio/src/targets/st16/board.h`: G11P `NUM_TRIMS`, function-switch count, battery
  thresholds/divider (reuse the ST16 2S config), stick dead-zone.
- `radio/src/targets/st16/board.cpp`: branch module on/off, audio RST/MUTE, and init flow;
  **drop `flysky_gimbal_init()`** (sticks are pure analog).
- `radio/src/targets/st16/usb_descriptor.h`: `#elif defined(RADIO_G11P)` with
  `USB_NAME "G11P"` and product string.
- `radio/src/targets/st16/bsp_io.h` / `bsp_io.cpp`: G11P has **no PCA95xx**; add a
  `#if defined(RADIO_G11P)` direct-GPIO implementation (or `g11p_bsp_io.cpp`) mapping the
  `BSP_*` outputs (`BSP_INT_PWR`, `BSP_EXT_PWR`, `BSP_AUDIO_RST`, `BSP_PA_NMUTE`,
  `BSP_CHARGE_EN`, `BSP_PWR_LED`, `BSP_LCD_NRST`, `BSP_LCD_CS`) to the nets above.
- **LEDs**: single WS2812 chain, `LED_STRIP_LENGTH = 2` logical LEDs (4 physical in two
  parallel pairs); `LED_DATA_IN` (PA15); confirm `LED_DATA2` (PB13) role.
- **Audio**: VS1053B on SPI1 with `INVERTED_MUTE_PIN` (`PA_NMUTE`=PD5, `VS1053B_RST`=PD6);
  LM4890 speaker amp.
- New `radio/src/targets/st16/g11p_key_driver.cpp` and `g11p_switch_driver.cpp`, modelled
  on `pl18/nb4p_key_driver.cpp` / `nb4p_switch_driver.cpp` (see 3.4).
- `extflash_driver.*`: keep the QSPI implementation; do not port the stale `FLASH_SPI`/SPI6
  macros (optionally remove them from the ST16/G11P path).

### 3.3 Bootloader

Reuse the ST16 bootloader flow. Confirm the boot/loader key source on G11P
(ADC ladder like NB4P vs a GPIO).

### 3.4 Input strategy (keys / switches / trims / encoder)

Design decisions (confirmed):

- **No port extender** on G11P (unlike ST16's PCA9555/NCA9555). I2C is used only for touch
  (`TP_SDA`/`TP_SCL`) and `I2C1`; there is no I2C I/O expander, so `bsp_io` gets a
  direct-GPIO path.
- **G11P uses multi-level resistor-ladder inputs** (same class as NB4P), so plain hw_defs
  pin mapping is not sufficient and custom drivers are required:
  - The key board wires K1/K2 onto `K12_ADC` and K3/K4 onto `K34_ADC` (two buttons per line
    via differing resistors, e.g. 5.1K vs 10K).
  - `TR1_ADC` feeds a 5-way switch (`K1-5203UA-01`) through a resistor network.
  - The SIDE sheet even prints the 3-resistor ladder math (R1=5.1K, R2=10K, R3=20K, R0=5.1K
    -> 7 levels).
- **Single rotary encoder** via the common HAL (`ROTARY_ENCODER_*`): `W-A`=PG2, `W-B`=PG3,
  `W-KEY`=PG14 (push), with `ROTARY_ENCODER_NAVIGATION`. EdgeTX's common layer supports
  exactly one encoder; the NB4P precedent is that extra hardware encoders (its `BM2A/BM2B`
  on PB15/PC13) are left unmapped, and G11P exposes only one (`W-A/W-B/W-KEY`).
- **`SW1` is a digital switch** on PH4 (the `_ADC` suffix is only the connector net name;
  PH4 is not ADC-capable on STM32H750, the same quirk as ST16's unused `BAT2_ADC`).
- **K5/VR1/SW1 are distinct inputs**, not swappable modules (they are only drawn separately
  in the schematic). They must be treated as proper inputs.

MCU input inventory (from the P2 MCU sheet):

| Net | Pin | Electrical | Kind | Logical (inferred, TODO) |
|---|---|---|---|---|
| `TH_ADC` | PA0 | ADC | stick | Throttle (TH) |
| `ST_ADC` | PA1 | ADC | stick | Steering (ST) |
| `VR2A` | PC2 | ADC | pot | Pot VR2 |
| `K12_ADC` | PB0 | ADC 2-level ladder | keys K1/K2 | 2 keys - TODO |
| `K34_ADC` | PB1 | ADC 2-level ladder | keys K3/K4 | 2 keys - TODO |
| `K6A` | PC7 | GPIO | digital key | TODO |
| `K7A` | PA8 | GPIO | digital key | TODO |
| `W-A` / `W-B` | PG2/PG3 | GPIO | encoder | Navigation |
| `W-KEY` | PG14 | GPIO | key | Encoder push (K11?) |
| `SW1_ADC` | PH4 | GPIO | digital switch | SW1 (2POS/toggle) |
| `SW2_ADC` | PA6 | ADC | analog switch | SW2 - TODO type |
| `SW3_ADC` | PA7 | ADC | analog switch | SW3 - TODO type |
| `SW4_ADC` | PC3 | ADC | analog switch | SW4 - TODO type |
| `TR1_ADC` | PF8 | ADC (5-way) | trim | TR1 - TODO |
| `TR2_ADC` | PF9 | ADC | trim | TR2 - TODO |
| `TR3_ADC` | PA2 | ADC | trim | TR3 - TODO |
| `TR4_ADC` | PA3 | ADC | trim | TR4 - TODO |
| `TR_ADC` | PC5 | ADC | trim | TR - TODO |
| `MOTOR_PWM` | PB5 | PWM | haptic | Vibration motor |

Notes:
- `K5`, `VR1`, `K11` do not appear as separate MCU nets (only `K12_ADC`, `K34_ADC`,
  `SW1_ADC`, `VR2A`, `K6A`/`K7A`, `W-KEY`); they either alias onto these lines or live on a
  sub-board - TODO to confirm on hardware.
- Logical names/types for K/SW/TR/VR are inferred from the schematic + NB4P conventions and
  are TODO until confirmed from the product.

Driver implementation:
- `g11p_key_driver.cpp`: `readKeys()` decodes `K12_ADC`/`K34_ADC` into 4 keys (thresholds
  from the ladder resistor values, in the NB4P `getAnalogValue()` style) and handles the
  `K6A`/`K7A` GPIO keys; `readTrims()` handles the `TRx_ADC` lines.
- `g11p_switch_driver.cpp`: `SW1` digital (PH4) plus analog `SW2/3/4`; switch names must
  align with `switch_config.py`.
- `g11p_bsp_io`: direct-GPIO (no PCA95xx) implementation for the `BSP_*` outputs.
- Sticks (`TH`/`ST`), pot (`VR2`) and the encoder use the normal HAL paths.

---

## 4. Hardware definitions and analog inputs

- `radio/src/targets/st16/hal.h` `ADC_*` macros define the ADC channels; these are parsed
  by `radio/util/hw_defs/hal_adc.py`.
- Add `"g11p"` entries to:
  - `radio/util/hw_defs/hal_keys.py` (key/trim labels)
  - `radio/util/hw_defs/switch_config.py`
  - `radio/util/hw_defs/pot_config.py`
  - `radio/util/hw_defs/legacy_names.py` (analog input names/labels)
  Model them on `nb4p` (surface) and `st16`, adjusted to the G11P nets and the input map in
  3.4. The switch names in `switch_config.py` must match the names returned by
  `g11p_switch_driver.cpp`.

---

## 5. Companion and storage

### 5.1 Storage / YAML

- `radio/src/storage/yaml/CMakeLists.txt`: add G11P branch emitting
  `storage/yaml/yaml_datastructs_g11p.cpp`.
- `radio/src/datastructs.h`: G11P shares `PCBST16` ModelData size unless it diverges.

### 5.2 Companion — full board support

A first-class Companion board type is added, not the NB4P "firmware-only" shortcut.

**`companion/src/firmwares/boards.h`**

- Add `BOARD_FLYSKY_G11P` to `Board::Type`, **appended at the end** (after
  `BOARD_HELLORADIOSKY_V14`, before `BOARD_TYPE_COUNT`) so existing enum values stay stable.
- Add `inline bool IS_FLYSKY_G11P(Board::Type board)`.
- Add G11P to the family helpers where ST16 appears:
  - `IS_FAMILY_HORUS_OR_T16` (color LCD/GUI, menus, model labels/list)
  - `IS_STM32`
  - `IS_STM32H7` (Sensors count, SPort baud rate)
  - Optionally introduce `IS_FAMILY_ST16 = IS_FLYSKY_ST16 || IS_FLYSKY_G11P` and reuse it.

**`companion/src/firmwares/boards.cpp`**

| Function | Change |
|---|---|
| `getFourCC` | Give G11P a **unique** magic (e.g. `0x4F78746F`; `0x4D`/`0x4E` are taken). Do **not** share ST16's `0x4C78746F`, otherwise air/surface model files become cross-compatible. |
| `getEEpromSize` | Add G11P to the `return 0` (Horus-class) group (also covered by the `default`). |
| `getFlashSize` | Add G11P to `FSIZE_HORUS` (8 MB, same as ST16). |
| `getCapability` `LcdHeight` | Add `IS_FLYSKY_G11P` to the `320` branch (same as ST16). |
| `getCapability` `LcdWidth` | Add `IS_FLYSKY_G11P` to the `480` branch. |
| `getCapability` **`Surface`** | **Critical**: `return IS_RADIOMASTER_MT12(board) || IS_FLYSKY_G11P(board);` — drives surface channel order, trim switches, flight-mode and virtual-joystick UI. |
| `getCapability` `HasLedStripGPIO` | Decide; ST16 is currently excluded, so leave G11P excluded unless Companion needs the LED-strip options. |
| `getDefaultInternalModules` | `BOARD_FLYSKY_G11P` → `MODULE_TYPE_FLYSKY_AFHDS3`. |
| `getBattRange` | Add G11P; same 2S platform as ST16 (`BR(70,86,80)`) pending schematic confirmation. |
| `getBoardName` | `case BOARD_FLYSKY_G11P: return "FlySky G11P";` |
| default `getCapability` | Falls through to `getBoardJson(board)` → inputs/pots/sliders/switches/keys/trims read from `g11p.json`. |

**`companion/src/firmwares/opentx/opentxinterface.cpp`**

- Register the firmware after the ST16 entry:

  ```cpp
  /* FlySky G11P board */
  firmware = new OpenTxFirmware(FIRMWAREID("g11p"), Firmware::tr("FlySky G11P"), BOARD_FLYSKY_G11P);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption(opt_bt);
  addOpenTxRfOptions(firmware, FLEX + AFHDS3);
  registerOpenTxFirmware(firmware);
  ```

- Add G11P to the ST16 branches in `OpenTxFirmware::getCapability`:
  - `HasAuxSerialMode` (ST16 is excluded → add G11P)
  - `RotaryEncoderNavigation` (ST16 included → add G11P)
  - `BacklightLevelMin` (ST16 included → add G11P)
- `Sensors`/`SportMaxBaudRate` are covered via `IS_STM32H7`; `Heli` via `Surface`;
  `HasBluetooth` via `IS_FAMILY_HORUS_OR_T16`.

**Other Companion sources**

- `companion/src/firmwares/generalsettings.cpp`: Bluetooth name → `"g11p"` for G11P.
- `companion/src/firmwares/moduledata.cpp`: add `!IS_FLYSKY_G11P(board)` to both PXX gating
  expressions (around lines 88/90).
- No changes needed under `companion/src/tests` (no board-list assertions).

### 5.3 Companion build system

- `companion/src/CMakeLists.txt` (flavour selection near line 365): add
  `elseif(PCB STREQUAL ST16 AND PCBREV STREQUAL G11P) set(FLAVOR g11p)` **before** the
  plain ST16 case.
- `tools/build-companion.sh`: add `g11p` to the `simulator_plugins` array so `g11p.json`
  is generated into the build tree and swept into `hwdefs.qrc` by
  `companion/util/generate_hwdefs_qrc.py`.
- `tools/build-common.sh`: the `g11p)` case (shared with the radio plan) makes
  `get_target_build_options g11p` resolve for the Companion build.
- **Dependency**: the embedded JSON comes from the radio build (`AddHardwareDefTarget` →
  `build/radio/src/g11p.json`). This requires the `"g11p"` entries in
  `radio/util/hw_defs/{hal_keys,switch_config,pot_config,legacy_names}.py`; without them
  `g11p.json` has no inputs/keys/switches/trims and Companion shows empty controls.

### 5.4 Translations

- The new `Firmware::tr("FlySky G11P")` string must be added to the 24 `companion_*.ts`
  files. Run the `companion_translations` target (Qt `lupdate`) or hand-add to each file;
  `companion_en.ts` already carries the ST16 entry under the `Firmware` context.

### 5.5 Compatibility decision (open)

- A **unique fourCC** means ST16 model files will not directly load on G11P (correct given
  air vs surface). If shared models are desired, reuse ST16's `0x4C78746F` instead. This is
  a product decision.

---

## 6. Verification

The firmware build runs in the pre-installed container image
`ghcr.io/edgetx/edgetx-dev:2.11` (Arm GNU Toolchain 14.2 on `PATH`).

Reference build scripts on the host under `/edgetx` (used by the nightly/release machinery):

- `/edgetx/build/2.11-build-gh-release.sh` - target + language build; reads `FLAVOR`
  (target) and `LANG` (translations), calls `get_target_build_options`, writes
  `etx-*.bin`/`.uf2` to `/build/output`.
- `/edgetx/build/2.11-build-all-release.sh` - exports `LANG`/`FLAVOR` then calls the above.
- `/edgetx/start-build.sh`, `/edgetx/build-release.sh` - cron/release wrappers showing the
  `docker run -v <repo>:/edgetx -v /edgetx/build:/build` mount pattern.

### 6.1 Build commands

Target + language build (mirrors the release flow):

```sh
cd /home/clli/edgetx
docker run -t --rm \
  -e GITHUB_REF='refs/heads/richardclli/fs-g11p-2.11' \
  -v "/home/clli/edgetx:/edgetx" \
  -v "/edgetx/build:/build" \
  -w /edgetx \
  --entrypoint /bin/bash \
  ghcr.io/edgetx/edgetx-dev:2.11 \
  -lc 'export LANG=EN FLAVOR=st16; /build/2.11-build-gh-release.sh'
```

Output: `/edgetx/build/output/etx-<tag>-st16-en.uf2` (host).

CI-equivalent (repo script, EN default) - same as `.github/workflows/actions.yml`:

```sh
docker run -t --rm -e FLAVOR=st16 -e EDGETX_VERSION_SUFFIX=g11p-dev \
  -v "/home/clli/edgetx:/src" -w /src --entrypoint /bin/bash \
  ghcr.io/edgetx/edgetx-dev:2.11 -lc './tools/build-gh.sh'
```

Output: `/home/clli/edgetx/st16-<sha>.uf2`. Select a language with
`-e EXTRA_OPTIONS="-DTRANSLATIONS=CN"` (appended to `COMMON_OPTIONS`).

Once the G11P target exists, add the `g11p)` case to `tools/build-common.sh`
(`-DPCB=ST16 -DPCBREV=G11P`) and build with `FLAVOR=g11p`.

Notes:
- `GITHUB_REF` only sets `EDGETX_VERSION_SUFFIX`; the release script uses
  `git describe --tags` for the filename, so the repo must have reachable tags (it does).
  Otherwise use the `tools/build-gh.sh` route.
- Builds run as root in the container; artifacts land in the repo `build/` and the mounted
  output dir (both ignored).

### 6.2 Verification steps

1. On this branch, build **st16** with the command above to confirm the toolchain/mounts
   work before starting target work.
2. Configure/build G11P: `-DPCB=ST16 -DPCBREV=G11P ... && make firmware`
   (or the docker commands above with `FLAVOR=g11p`).
3. `make hardware_defs`, `make yaml_inputs` to validate the generated hardware definitions.
4. Flash and bring up: boot, LCD init/touch, SD, audio (VS1053B), internal module bind,
   keys/switches/trims/sticks, battery/charge, USB, LEDs, backlight.
5. Companion: build with `g11p` and verify the board appears as "FlySky G11P" (surface UI,
   channel order ST/TH), loads `g11p.json`, and shows the expected inputs/keys/switches/trims.
6. CI: add `g11p` to commit-tests and the build matrix in `.github/workflows/actions.yml`.

---

## 7. Risks / open items

- Input map TODOs (see 3.4): logical functions of K1-K4 / K6 / K7 / K5 / K11; switch
  names/types for SW1-SW4; trim axes and whether TR1 is a true 5-way; purpose of `TR_ADC`;
  whether `K5` / `VR1` exist and where they wire. Exact K12/K34 ladder resistor values to
  derive thresholds.
- **Resolved**: LCD is 320x480 portrait; no external module bay / Bluetooth / wireless
  charger / S.Port / IMU; trainer port exposed as the external module UART (UART7);
  touch reuses the PL18 FlySky driver; battery reuses the ST16 2S config; unique G11P
  fourCC.
- `MS0621-32TFL11E` panel timing/polarity values for the G11P LTDC config.
- `LED_DATA2` (PB13) role (second parallel LED group vs alternate data line).
- Audio mute polarity (`INVERTED_MUTE_PIN`) on G11P.
- Bootloader entry key/button on G11P.

---

## 8. Task checklist

**Commit/push policy:** when each checklist item is finished and verified, stage only the
files for that item, commit with a conventional message (`feat(g11p): ...`, `fix(g11p): ...`,
`docs(g11p): ...`, matching repo style), and `git push origin HEAD` on
`richardclli/fs-g11p-2.11` before starting the next item. Do not batch unrelated items into
one commit, and do not leave the tree dirty between tasks. If a task spans radio + Companion
+ tooling, split it into focused commits (e.g. one per subsystem) and push each.

1. [x] Extract and save `st16-vs-g11p-pinout.md`. -> commit & push (done: `95e032d352`)
2. [x] Radio: `targets/st16/CMakeLists.txt` `PCBREV` branch + `RADIO_G11P` (cmake
   configure verified for `-DPCBREV=G11P` and plain ST16). -> commit & push
3. [x] Radio: `hal.h` / `board.h` / `board.cpp` / `usb_descriptor.h` G11P branches
   (LCD 320x480 portrait, touch via PL18 driver, `EXTMODULE` on UART7, no flysky gimbal,
   battery reuse). New `hal_g11p.h` included from `hal.h` for `RADIO_G11P`; cmake configure
   verified for both G11P and ST16 (full compile after tasks 4/6). -> commit & push
4. [x] Radio: `bsp_io` direct-GPIO (no expander) path (`bsp_io.h` G11P branch +
   `g11p_bsp_io.cpp`). cmake configure verified. **Superseded by section 9** (use PL18
   direct-GPIO style, no BSP driver). -> commit & push
5. [~] Confirm the logical input map (3.4 TODOs): inferred from schematic + NB4P and
   marked TODO; to be confirmed on hardware. -> commit & push
6. [x] Radio: `g11p_key_driver.cpp` / `g11p_switch_driver.cpp` (custom ADC-ladder decode)
   + `hal_g11p.h` ADC input map. G11P firmware compiles (docker build `FLAVOR` g11p).
   -> commit & push
7. [x] Tooling registration (`build-common.sh`, `build-flysky.py`, generators,
   `build-companion.sh`, `fw.json`). `FLAVOR=g11p` builds via `build-gh.sh`. CI matrix
   (`.github/workflows/actions.yml`) pending: push token lacks `workflow` scope.
   -> commit & push
8. [x] `hw_defs` entries (`switch_config`, `pot_config`, `legacy_names`, `hal_adc`
   `MAX_RAWS`) — required by both radio and Companion. -> commit & push
9. [x] Companion: `BOARD_FLYSKY_G11P` enum + `IS_FLYSKY_G11P` + family helpers (`boards.h`).
   -> commit & push
10. [x] Companion: `boards.cpp` (fourCC `0x4F78746F`, flash/eeprom, LCD 480x320 readout,
    Surface, default internal module AFHDS3, batt range, name). -> commit & push
11. [x] Companion: `opentxinterface.cpp` firmware registration + ST16-branch capabilities.
    -> commit & push
12. [x] Companion: `generalsettings.cpp` (BT name `g11p`) / `moduledata.cpp` branches.
    -> commit & push
13. [x] Companion build: `companion/src/CMakeLists.txt` flavour + `build-companion.sh` plugin
    (added in task 7). -> commit & push
14. [x] Translations: `lupdate` run over `companion_*.ts` (adds "FlySky G11P" + location
    updates). -> commit & push
15. [x] Storage/YAML (`yaml_datastructs_g11p.cpp`) generated and wired for `RADIO_G11P`.
    -> commit & push
16. [~] Build verified: G11P firmware (docker `FLAVOR=g11p` -> `g11p-*.uf2`), g11p
    `libsimulator`, and Companion (`companion211`) all build. Hardware flash/bring-up
    still pending. -> commit & push

17. [ ] Apply section 9 fix — delete `g11p_bsp_io.cpp`, PL18-style `bsp_io.h` stub, direct
    GPIO in `board.cpp` / `lcd_driver.h`; rebuild G11P firmware. -> commit & push

**Outstanding:** `.github/workflows/actions.yml` CI matrix needs `g11p` added, but the
push token lacks `workflow` scope; apply manually or with a suitably-scoped token. Hardware
bring-up items remain (input map, panel timings, etc. - see section 7).

---

## 9. Fix — replace the G11P BSP driver with direct GPIO (PL18 style)

The initial G11P implementation added `g11p_bsp_io.cpp` and a `BSP_*` mapping. That is
wrong: G11P has no I/O expander, and EdgeTX already has a direct-GPIO pattern in the PL18
target. Reference: `radio/src/targets/pl18/bsp_io.h` is a **header-only stub**

```cpp
inline SwitchHwPos bsp_get_switch_position(const stm32_switch_t *sw, SwitchCategory cat, uint8_t idx)
{ return SWITCH_HW_MID; }
```

and all real I/O is done with **direct GPIO**: macros in `hal.h` (`LCD_NRST_GPIO`,
`AUDIO_RST_GPIO`, `AUDIO_MUTE_GPIO`, `UCHARGER_EN_GPIO`, module power, LED strip) plus
`gpio_init`/`gpio_write` in `board.cpp`. There is **no BSP driver**.

Fix steps:

1. **Delete** `radio/src/targets/st16/g11p_bsp_io.cpp`.
2. **`radio/src/targets/st16/bsp_io.h`** — make the `RADIO_G11P` branch a PL18-style
   header-only stub (no enum, no `bsp_io_init`/`bsp_output_*`), only the inline
   `bsp_get_switch_position()` (kept so `boards/generic_stm32/switches.cpp` still compiles).
3. **`radio/src/targets/st16/CMakeLists.txt`** — for G11P compile **no** bsp source
   (`BSP_IO_SRC` empty). Keep `g11p_switch_driver.cpp` in the firmware `board` library.
4. **`radio/src/targets/st16/board.cpp`** — remove the ST16 expander calls from the G11P
   path (`bsp_io_init()`, `bsp_output_set(BSP_PWR_LED)`), and instead `gpio_init(...)` the
   direct outputs (`LCD_NRST_GPIO`, `LCD_SPI_CS_GPIO`, `CHARGE_EN_GPIO`, `AUDIO_RST_GPIO`,
   `AUDIO_MUTE_GPIO`). The G11P `INTERNAL/EXTERNAL_MODULE_ON/OFF` and
   `audio_set_rst_pin`/`audio_set_mute_pin` already use direct GPIO.
5. **`radio/src/targets/st16/lcd_driver.h`** — for G11P define `LCD_NRST_HIGH/LOW` and
   `LCD_CS_HIGH/LOW` with `gpio_set/gpio_clear` on `LCD_NRST_GPIO` / `GPIO_PIN(GPIOH, 8)`
   instead of the `bsp_output_set(BSP_LCD_*)` versions.
6. **`radio/src/targets/st16/hal_g11p.h`** — keep the direct GPIO macros
   (`AUDIO_RST_GPIO`, `AUDIO_MUTE_GPIO`, `CHARGE_EN_GPIO`, `LCD_NRST_GPIO`,
   `LCD_SPI_CS_GPIO`) and drop the unused expander `USE_EXTI9_5_IRQ`.
7. Rebuild the G11P firmware and confirm no `bsp_*`/`BSP_*` references remain on the G11P
   compile path. Commit as `fix(g11p): use direct GPIO I/O instead of a BSP driver`.
