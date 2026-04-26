# STM32 Platform Support

This page lists which EdgeTX-supported radios use each STM32 processor family, along with the support status for each platform generation.

---

## STM32F2

!!! warning "End of Support"
    Support for STM32F2 based radios ends with EdgeTX v2.11.

### BetaFPV

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| LR3Pro | 512KB | 128KB | B&W (OLED) | 128×64 |

### FrSky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| X7 | 512KB | 128KB | B&W | 128×64 |
| X9D | 512KB | 128KB | Greyscale | 212×64 |
| X9D+ | 512KB | 128KB | Greyscale | 212×64 |
| X9Lite | 512KB | 128KB | B&W | 128×64 |
| X9Lite S | 512KB | 128KB | B&W | 128×64 |
| XLite | 512KB | 128KB | B&W | 128×64 |
| XLite S | 512KB | 128KB | B&W | 128×64 |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| T12 | 512KB | 128KB | B&W | 128×64 |
| TPro | 512KB | 128KB | B&W | 128×64 |
| TLite | 512KB | 128KB | B&W | 128×64 |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| T8 | 512KB | 128KB | B&W (OLED) | 128×64 |
| TX12 | 512KB | 128KB | B&W | 128×64 |

---

## STM32F4

!!! info "Continued Support"
    Support for existing STM32F4 based radios will continue (i.e. will be supported in EdgeTX v2.12 onwards), but no new handsets will be added.

!!! warning "512KB flash constraint"
    Radios marked <span class="flash-warn">⚠ 512KB</span> have only 512KB of flash, which may limit which features can be included in the standard firmware build.

!!! note "SDRAM"
    Colour screen radios use an STM32F429 and include 8MB of external SDRAM in addition to the internal SRAM shown below.

### Fatfish

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| F16 | 2MB | 256KB + 8MB | Colour | 480×272 |

### Flysky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| EL18 | 2MB | 256KB + 8MB | Colour | 320×480 |
| NB4+ | 2MB | 256KB + 8MB | Colour | 320×480 |
| NV14 | 2MB | 256KB + 8MB | Colour | 320×480 |
| PL18 | 2MB | 256KB + 8MB | Colour | 480×320 |
| PL18EV | 2MB | 256KB + 8MB | Colour | 480×320 |
| PL18U | 2MB | 256KB + 32MB | Colour | 480×320 |

### FrSky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| X7 ACCESS | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 |
| X9E | <span class="flash-warn">⚠ 512KB</span> | 192KB | Greyscale | 212×64 |
| X9D+2019 | <span class="flash-warn">⚠ 512KB</span> | 192KB | Greyscale | 212×64 |
| X10 | 2MB | 256KB + 8MB | Colour | 480×272 |
| X10 EXPRESS | 2MB | 256KB + 8MB | Colour | 480×272 |
| X12S | 2MB | 256KB + 8MB | Colour | 480×272 |

### HelloRadioSky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| V14 | 1MB | 192KB | B&W (OLED) | 128×64 |
| V16 | 2MB | 256KB + 8MB | Colour | 480×272 |

### iFlight

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| Commando8 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| Bumblebee | 1MB | 192KB | B&W (OLED) | 128×64 |
| T12 MAX | 1MB | 192KB | B&W | 128×64 |
| T14 | 1MB | 192KB | B&W (OLED) | 128×64 |
| T15 | 2MB | 256KB + 8MB | Colour | 480×320 |
| T16 | 2MB | 256KB + 8MB | Colour | 480×272 |
| T18 | 2MB | 256KB + 8MB | Colour | 480×272 |
| T20 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W (OLED) | 128×64 |
| T20 V2 | 1MB | 192KB | B&W (OLED) | 128×64 |
| TPro S | 1MB | 192KB | B&W (OLED) | 128×64 |
| TPro V2 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W (OLED) | 128×64 |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| Boxer | 1MB | 192KB | B&W | 128×64 |
| GX12 | 1MB | 192KB | B&W (OLED) | 128×64 |
| MT12 | 1MB | 192KB | B&W | 128×64 |
| Pocket | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 |
| TX12 MK2 | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 |
| TX16S | 2MB | 256KB + 8MB | Colour | 480×272 |
| Zorro | <span class="flash-warn">⚠ 512KB</span> | 192KB | B&W | 128×64 |

---

## STM32H7

!!! note "Preliminary Support"
    Preliminary support for some STM32H7 radios began in EdgeTX v2.11. EdgeTX v2.12 is considered the more stable release for most STM32H7 based radios.

!!! note "SDRAM"
    All STM32H7 radios include 8MB of external SDRAM in addition to the ~1MB of internal SRAM.

### Flysky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| ST16 | 8MB | 1MB + 8MB | Colour | 480×320 |
| PA01 | 8MB | 1MB + 8MB | Colour | 320×240 |

### HelloRadioSky

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| V12 | 8MB | 1MB + 8MB | Colour | 320×240 |

### Jumper

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| T15Pro | 16MB | 1MB + 8MB | Colour | 480×320 |

### RadioMaster

| Radio | Flash | SRAM | Screen | Resolution |
|-------|-------|------|--------|------------|
| TX15 | 16MB | 1MB + 8MB | Colour | 480×320 |
| TX16S MK3 | 16MB | 1MB + 8MB | Colour | 800×480 |
