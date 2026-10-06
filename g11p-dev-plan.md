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
- **Inputs are surface-radio style (NB4P-like)**: `TH_ADC`/`ST_ADC` sticks, `SW1-4_ADC`
  + `VR2A`, `K12_ADC`/`K34_ADC` keys, `TR1-4_ADC` trims.
- **PPM moved** PE5/PE6 -> PF6/PF7; PE5/PE6 become RF power-on / touch interrupt.
- **SPORT UART5 (PB12/PB13)** is gone; PB12/PB13 are RF DFU / LED data.
- **Touch** moves to PB9 (SDA) / PB8 (SCL) / PE6 (INT) / PI9 (reset); the old ST16
  `SENSOR_*` (IMU) I2C lines on PB7/PB8 are not present the same way.
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
  SPI NOR bank, ADC key/switch/trim channels, battery, PPM, trainer, LED strip, haptic/motor).
- `radio/src/targets/st16/board.h`: G11P `NUM_TRIMS`, function-switch count, battery
  thresholds/divider, stick dead-zone.
- `radio/src/targets/st16/board.cpp`: branch module on/off, audio RST/MUTE, and init flow.
- `radio/src/targets/st16/usb_descriptor.h`: `#elif defined(RADIO_G11P)` with
  `USB_NAME "G11P"` and product string.
- `radio/src/targets/st16/bsp_io.h` / `bsp_io.cpp`: G11P has **no PCA95xx**; add a
  `#if defined(RADIO_G11P)` direct-GPIO implementation (or `g11p_bsp_io.cpp`) mapping the
  `BSP_*` outputs (`BSP_INT_PWR`, `BSP_EXT_PWR`, `BSP_AUDIO_RST`, `BSP_PA_NMUTE`,
  `BSP_CHARGE_EN`, `BSP_PWR_LED`, `BSP_LCD_NRST`, `BSP_LCD_CS`) to the nets above.
- New `radio/src/targets/st16/g11p_key_driver.cpp` and `g11p_switch_driver.cpp`, modelled
  on `pl18/nb4p_key_driver.cpp` / `nb4p_switch_driver.cpp`, using the G11P ADC ladder layout
  (`K12_ADC`/`K34_ADC`, `SW1-4_ADC`, `TR1-4_ADC`, `VR2A`).
- `extflash_driver.*`: keep the QSPI implementation; do not port the stale `FLASH_SPI`/SPI6
  macros (optionally remove them from the ST16/G11P path).

### 3.3 Bootloader

Reuse the ST16 bootloader flow. Confirm the boot/loader key source on G11P
(ADC ladder like NB4P vs a GPIO).

---

## 4. Hardware definitions and analog inputs

- `radio/src/targets/st16/hal.h` `ADC_*` macros define the ADC channels; these are parsed
  by `radio/util/hw_defs/hal_adc.py`.
- Add `"g11p"` entries to:
  - `radio/util/hw_defs/hal_keys.py` (key/trim labels)
  - `radio/util/hw_defs/switch_config.py`
  - `radio/util/hw_defs/pot_config.py`
  - `radio/util/hw_defs/legacy_names.py` (analog input names/labels)
  Model them on `nb4p` (surface) and `st16`, adjusted to the G11P nets.

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

1. Configure/build: `cmake -DPCB=ST16 -DPCBREV=G11P ... && make firmware`
   (or via `tools/build-common.sh`).
2. `make hardware_defs`, `make yaml_inputs` to validate the generated hardware definitions.
3. Flash and bring up: boot, LCD init/touch, SD, audio (VS1053B), internal module bind,
   keys/switches/trims/sticks, battery/charge, USB, LEDs, backlight.
4. Companion: build with `g11p` and verify the board appears as "FlySky G11P" (surface UI,
   channel order ST/TH), loads `g11p.json`, and shows the expected inputs/keys/switches/trims.
5. CI: add `g11p` to commit-tests and the build matrix in `.github/workflows/actions.yml`.

---

## 7. Risks / open items

- Exact G11P ADC ladder resistor values and channel assignments (keys/switches/trims)
  need hardware or deeper schematic reading.
- Presence/absence of external module bay, trainer, SPORT, Bluetooth, wireless charge, IMU
  and their pin assignments (the relevant ST16 pins are repurposed on G11P).
- Touch controller model/I2C address (moved to PB9/PB8, INT PE6).
- LCD resolution/orientation (schematics suggest 480x320, same RGB pinout) and audio mute
  polarity.
- Companion fourCC: use a unique G11P magic (default) or share ST16's (see 5.5).

---

## 8. Task checklist

1. [x] Extract and save `st16-vs-g11p-pinout.md`.
2. [ ] Radio: `targets/st16/CMakeLists.txt` `PCBREV` branch + `RADIO_G11P`.
3. [ ] Radio: `hal.h` / `board.h` / `board.cpp` / `usb_descriptor.h` G11P branches.
4. [ ] Radio: `bsp_io` direct-GPIO (no expander) path.
5. [ ] Radio: `g11p_key_driver.cpp` / `g11p_switch_driver.cpp`.
6. [ ] Tooling + CI registration (`build-common.sh`, `build-flysky.py`, generators,
   `.github/workflows/actions.yml`, `fw.json`).
7. [ ] `hw_defs` entries (`hal_keys`, `switch_config`, `pot_config`, `legacy_names`) —
   required by both radio and Companion.
8. [ ] Companion: `BOARD_FLYSKY_G11P` enum + `IS_FLYSKY_G11P` + family helpers (`boards.h`).
9. [ ] Companion: `boards.cpp` (fourCC, flash/eeprom, LCD, Surface, default internal module,
   batt range, name).
10. [ ] Companion: `opentxinterface.cpp` firmware registration + ST16-branch capabilities.
11. [ ] Companion: `generalsettings.cpp` / `moduledata.cpp` G11P branches.
12. [ ] Companion build: `companion/src/CMakeLists.txt` flavour + `build-companion.sh` plugin.
13. [ ] Translations: run `companion_translations` (lupdate).
14. [ ] Storage/YAML (`yaml_datastructs_g11p.cpp`).
15. [ ] Build, flash, hardware + Companion validation.
