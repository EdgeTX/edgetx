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
firmware-write drive are deliberately not enabled for this target. Installation
and rollback require a separately verified, original-bootloader-compatible
application uploader; no uploader or stock firmware is included in this PR.

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
boundaries and bay UART/DMA vectors, eight standalone host suites, hardware
schema/template generation, and a TX12 Mark II firmware regression using its
normal USE_FW_LTO=YES build setting. See tools/mambo-tests/README.md for the
reproducible tests; no private calibration files or captured firmware required.

Keep PR #7907 experimental/draft. Receiver failsafe, long-duration stability,
charging/off-state behavior, physical emergency-shutdown bay checks, faster
CRSF negotiation/interoperability and Companion/release support remain open.
This is bench evidence, not flight validation. No nightly or release target
is added, and no private calibration, captured binaries or RF research tooling
belongs in the patch.
