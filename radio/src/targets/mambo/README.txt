Experimental TBS Mambo support
=============================

This is an application-only STM32F407 target using the original TBS bootloader.
Internal Tracer RF is not started, linked or updated on mambo-support. Its
experimental integration belongs to the separate mambo-tracer branch.

Default build
-------------
Use the normal EdgeTX build dependencies and ARM toolchain. From the repo root:

  cmake --preset mambo -DLUA=ON -DLUA_COMPILER=ON -DLUA_MIXER=ON
  cmake --build build/mambo --target firmware

External-bay CRSF support is always included; no additional build option or
separate firmware profile is required. Set Model Setup -> External RF to CRSF
to enable the bay, or Off to leave it unpowered. Only Off and CRSF are supported;
other persisted/imported protocols are rejected and cannot enable bay power. The
working external path is USART2, DMA1 Stream6, with both inverter controls low
for normal CRSF. The built-in Tracer remains disabled.

Installation and calibration
----------------------------
The application starts at 0x0800C000 and must stay below 0x080C0000. The linker
also reserves shared SRAM and the first 1 KiB of CCM. Firmware images must not
contain a bootloader or internal RF payload. Generic DFU flashing and a USB
firmware-write drive are deliberately not enabled for this target. The Mambo
flash target instead uses the original TBS USB HID controller bootloader:

  cmake --build build/mambo --target flash-plan
  cmake --build build/mambo --target flash-probe

The first command builds and verifies an offline image plan without opening
USB; the second queries controller identity after PAGE-held USB connection.
Then set MAMBO_FLASH_SERIAL and MAMBO_FLASH_SHA256 from those results and run
the flash target. See tools/mambo-flash/README.txt for complete instructions,
dependencies, local transfer logs and stock-recovery precautions. The tool
checks the selected controller and image before application erase and never
updates the bootloader/internal RF. No stock firmware or private data is bundled.

Preserve the original firmware, SD contents and the radio's own calibration
backup before testing. The direct Hall backend can READ /CROSSFIRE/EEPROM.bin
(604 bytes) and validates its file, angle and corner checksums before using the
factory correction. It never creates, repairs or overwrites that file. Only
the currently supported format has been checked; arbitrary/other-version
backups are not guaranteed compatible. Without valid factory calibration,
the provisional projection is not proof of correct full-travel geometry.

Run ordinary EdgeTX stick calibration and select the correct stick Mode. Mode 2
uses the left vertical axis for throttle; default TAER channel order does not
select stick Mode. A throttle-idle warning was resolved on the test radio by
selecting Mode 2; the warning was not disabled or relaxed in firmware.

Radio Settings -> Analogs: PAGE cycles through Mambo Hall L/R and Mambo Bench.
The default build also has Mambo External (power, TX/RX, parsed frames and
errors). Cal SD OK means the factory backup passed validation, not that the
user's EdgeTX calibration or radio Mode is necessarily correct.

Validation and limits (2026-10-17)
--------------------------------
On one physical Mambo, the owner confirmed normal startup, working encoder and
Hall sticks with the RF-disabled b618ccbe89 build. Its separate external-CRSF
build was subsequently flashed with a matching terminal checksum and the
owner confirmed that the external bay works. That already-working bay path
is now included in the default Mambo build, with model-level Off/CRSF control.
The unified default 8773d9a5f7 build was then flashed successfully (478368 bytes,
matching terminal checksum, zero retries). After receiving a regression checklist
covering battery-only startup, encoder/Hall inputs, model Off/CRSF bay behavior,
receiver channels, shutdown power and repeated cold boots, the owner reported
"Everything looks good". This is an owner-reported overall bench pass; individual
measurements and an exact cold-boot count were not provided. Direct Hall and
APB2/TIM10 polling are retained. Earlier internal-RF diagnostic results are
separate evidence, not testing of these standalone applications.

Local verification covers the default Mambo firmware, application memory
boundaries and bay UART/DMA vectors, nine standalone host suites, hardware
schema/template generation, and a TX12 Mark II firmware regression using its
normal USE_FW_LTO=YES build setting. See tools/mambo-tests/README.md for the
reproducible tests; no private calibration files or captured firmware required.

Keep PR #7907 experimental/draft. Receiver failsafe, long-duration stability,
charging/off-state behavior, physical emergency-shutdown bay checks, faster
CRSF negotiation/interoperability and Companion/release support remain open.
This is bench evidence, not flight validation. No nightly or release target
is added, and no private calibration, captured binaries or RF research tooling
belongs in the patch. The public application-only flash target is separate
from internal-RF research tooling. Its default CMake path completed a full
controller application transfer on one macOS/Mambo bench at d1277a6043, using
15 ms pacing with no retries and a matching terminal checksum. This verifies
transfer completion, not flash readback. After being asked to check battery-only
startup, encoder, sticks and external bay, the owner reported "Everything looks
good" for that flashed build. This is an overall post-flash bench report, not
individual measurements, repeated-boot evidence or flight validation.
Windows/Linux flashing remains unqualified. See tools/mambo-flash/README.txt
for the test scope.
