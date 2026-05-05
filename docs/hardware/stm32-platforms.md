# STM32 Platform Support

This page lists which EdgeTX-supported radios use each STM32 processor family, along with the support status for each platform generation.

---

## STM32F2

!!! warning "End of Support"
    Support for STM32F2 based radios ends with EdgeTX v2.11.

### BetaFPV

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| LR3Pro | 512KB | 128KB | B&W (OLED) | 128×64 | No | Yes |

### FrSky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| X7 | 512KB | 128KB | B&W | 128×64 | No | Yes |
| X9D | 512KB | 128KB | Greyscale | 212×64 | No | Yes |
| X9D+ | 512KB | 128KB | Greyscale | 212×64 | No | Yes |
| X9Lite | 512KB | 128KB | B&W | 128×64 | No | Yes |
| X9Lite S | 512KB | 128KB | B&W | 128×64 | No | Yes |
| XLite | 512KB | 128KB | B&W | 128×64 | No | Yes |
| XLite S | 512KB | 128KB | B&W | 128×64 | No | Yes |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| T12 | 512KB | 128KB | B&W | 128×64 | No | Yes |
| TPro | 512KB | 128KB | B&W | 128×64 | No | Yes |
| TLite | 512KB | 128KB | B&W | 128×64 | No | Yes |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| T8 | 512KB | 128KB | B&W (OLED) | 128×64 | No | Yes |
| TX12 | 512KB | 128KB | B&W | 128×64 | No | Yes |

---

## STM32F4

!!! info "Continued Support"
    Support for existing STM32F4 based radios will continue (i.e. will be supported in EdgeTX v2.12 onwards), but no new handsets will be added.

!!! warning "512KB flash constraint"
    Radios marked <span class="flash-warn">⚠ 512KB</span> have only 512KB of flash, which may limit which features can be included in the standard firmware build.

!!! note "SDRAM"
    Colour screen radios use an STM32F429 and include 8MB of external SDRAM in addition to the internal SRAM shown below.

### Fatfish

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| F16 | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |

### Flysky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| EL18 | 2MB | 256KB + 8MB | Colour | 320×480 | No | Yes |
| NB4+ | 2MB | 256KB + 8MB | Colour | 320×480 | Yes | No |
| NV14 | 2MB | 256KB + 8MB | Colour | 320×480 | No | Yes |
| PL18 | 2MB | 256KB + 8MB | Colour | 480×320 | Yes | No |
| PL18EV | 2MB | 256KB + 8MB | Colour | 480×320 | Yes | No |
| PL18U | 2MB | 256KB + 32MB | Colour | 480×320 | Yes | No |

### FrSky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| X7 ACCESS | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 | No | Yes |
| X9E | <span class="flash-warn">⚠ 512KB</span> | 192KB | Greyscale | 212×64 | No | Yes |
| X9D+2019 | <span class="flash-warn">⚠ 512KB</span> | 192KB | Greyscale | 212×64 | No | Yes |
| X10 | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |
| X10 EXPRESS | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |
| X12S | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |

### HelloRadioSky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| V14 | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| V16 | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |

### iFlight

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| Commando8 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 | No | Yes |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| Bumblebee | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| T12 MAX | 1MB | 192KB | B&W | 128×64 | No | Yes |
| T14 | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| T15 | 2MB | 256KB + 8MB | Colour | 480×320 | No | Yes |
| T16 | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |
| T18 | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |
| T20 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W (OLED) | 128×64 | No | Yes |
| T20 V2 | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| TPro S | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| TPro V2 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W (OLED) | 128×64 | No | Yes |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| Boxer | 1MB | 192KB | B&W | 128×64 | No | Yes |
| GX12 | 1MB | 192KB | B&W (OLED) | 128×64 | No | Yes |
| MT12 | 1MB | 192KB | B&W | 128×64 | No | Yes |
| Pocket | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 | No | Yes |
| TX12 MK2 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 | No | Yes |
| TX16S / TX16S MKII | 2MB | 256KB + 8MB | Colour | 480×272 | No | Yes |
| Zorro | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 | No | Yes |

---

## STM32H7

!!! note "Preliminary Support"
    Preliminary support for some STM32H7 radios began in EdgeTX v2.11. EdgeTX v2.12 is considered the more stable release for most STM32H7 based radios.

!!! note "SDRAM"
    All STM32H7 radios include 8MB of external SDRAM in addition to the ~1MB of internal SRAM.

### Flysky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| ST16 | 8MB | 1MB + 8MB | Colour | 480×320 | No | Yes |
| PA01 | 8MB | 1MB + 8MB | Colour | 320×240 | Yes | No |

### HelloRadioSky

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| V12 | 8MB | 1MB + 8MB | Colour | 320×240 | Yes | No |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| T15Pro | 16MB | 1MB + 8MB | Colour | 480×320 | Yes | No |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution | Built-in storage | SD card |
|-------|-------|------|--------|------------|------------------|---------|
| TX15 | 16MB | 1MB + 8MB | Colour | 480×320 | Yes | Yes |
| TX16S MK3 | 16MB | 1MB + 8MB | Colour | 800×480 | Yes | Yes |
