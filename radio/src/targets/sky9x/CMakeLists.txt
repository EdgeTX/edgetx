set(ARCH ARM)
set(LUA NO)
set(MCU cortex-m3)
set(SDCARD YES)
set(HAPTIC YES)
set(STORAGE EEPROM)
set(STORAGE_FORMAT RAW)
set(GUI_DIR 128x64)
set(BITMAPS_DIR 128x64)
set(NAVIGATION_TYPE 9x)
set(BITMAPS_TARGET 9x_bitmaps)
set(TARGET_DIR sky9x)
set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} 9x_bitmaps)
set(PPM_LIMITS_SYMETRICAL YES)
set(PXX1 YES)
set(AFHDS3 NO)
set(GHOST NO)
add_definitions(-DHARDWARE_EXTERNAL_MODULE)
add_definitions(-DDISABLE_MPM_UPDATE)

if(PCB STREQUAL 9XRPRO)
  add_definitions(-Dat91sam3s8 -DREVX)
  set(FLAVOUR 9xrpro)
  set(LINKER_SCRIPT targets/sky9x/sam3s8c_flash.ld)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    rtc_driver.cpp
    )
  set(OPENRC_BOOTLOADER bootflash8.lbm)
  set(CPU_TYPE_FULL ATSAM3S8C)  # for size report
elseif(PCB STREQUAL AR9X)
  add_definitions(-Dat91sam3s8 -DPCBAR9X -DSOFTWARE_VOLUME)
  set(FLAVOUR ar9x)
  include_directories(${THIRDPARTY_DIR}/xdk-asf-3.31.0/sam/drivers/twi)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../../${THIRDPARTY_DIR}/xdk-asf-3.31.0/sam/drivers/twi/twi.c
    i2c_driver.cpp
    rtc_ds3231_driver.cpp
    )
  set(LINKER_SCRIPT targets/sky9x/sam3s8c_flash.ld)
  set(OPENRC_BOOTLOADER bootflash8.lbm)
  set(CPU_TYPE_FULL ATSAM3S8C)  # for size report
else()
  set(FLAVOUR sky9x)
  set(PCBREV "REVC" CACHE STRING "PCB Revision (REVB/REVC)")
  set_property(CACHE PCBREV PROPERTY STRINGS "REVB" "REVC")
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    coproc_driver.cpp
    rotary_encoder_driver.cpp
    rtc_driver.cpp
    )
  set(OPENRC_BOOTLOADER bootflash4.lbm)
  if(PCBREV STREQUAL REVB)
    add_definitions(-DDREVB)
  elseif(PCBREV STREQUAL REVC)
    add_definitions(-DDREVC)
  else()
    message(SEND_ERROR "Unknown PCB Revision '${PCBREV}'")
  endif()
  add_definitions(-DCOPROCESSOR)
  set(LINKER_SCRIPT targets/sky9x/sam3s4c_flash.ld)
  set(CPU_TYPE_FULL ATSAM3S4C)  # for size report
endif()

add_definitions(-DPCBSKY9X -DPPM_PIN_SERIAL -DAUDIO_DUAL_BUFFER)
add_definitions(-DAUDIO -DVOICE -DRTCLOCK -DVARIO)
add_definitions(-DEEPROM_VARIANT=0)

set(GUI_SRC
  ${GUI_SRC}
  model_input_edit.cpp
  model_mix_edit.cpp
  model_display.cpp
  radio_diagkeys.cpp
  radio_diaganas.cpp
  view_telemetry.cpp
  view_about.cpp
  view_channels.cpp
  )

if(GVARS)
  set(GUI_SRC
    ${GUI_SRC}
    model_gvars.cpp
    )
endif()

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ../common/arm/loadboot.cpp
  board.cpp
  core_cm3.c
  board_lowlevel.c
  crt.c
  vectors_sam3s.c
  eeprom_driver.cpp
  lcd_driver.cpp
  pwr_driver.cpp
  joystick.cpp
  massstorage.cpp
  keys_driver.cpp
  usb/device/core/USBD_UDP.c
  usb/device/core/USBDDriver.c
  usb_driver.cpp
  usb/device/massstorage/MSDDriver.c
  usb/device/massstorage/MSDDStateMachine.c
  usb/device/massstorage/MSDLun.c
  usb/device/massstorage/MSDDriverDescriptors.c
  usb/device/massstorage/SBCMethods.c
  usb/device/hid-joystick/HIDDJoystickDriver.c
  usb/device/hid-joystick/HIDDJoystickDriverDescriptors.c
  usb/device/hid-joystick/HIDDJoystickInputReport.c
  usb/common/core/USBEndpointDescriptor.c
  usb/common/core/USBGenericRequest.c
  usb/common/core/USBFeatureRequest.c
  usb/common/core/USBInterfaceRequest.c
  usb/common/core/USBGetDescriptorRequest.c
  usb/common/core/USBSetAddressRequest.c
  usb/common/core/USBSetConfigurationRequest.c
  usb/common/core/USBConfigurationDescriptor.c
  usb/common/core/USBGenericDescriptor.c
  usb/common/hid/HIDIdleRequest.c
  usb/common/hid/HIDReportRequest.c
  MEDSdcard.c
  )

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  telemetry_driver.cpp
  )

set(TARGET_SRC
  ${TARGET_SRC}
  adc_driver.cpp
  pulses_driver.cpp
  audio_driver.cpp
  buzzer_driver.cpp
  haptic_driver.cpp
  sdcard_driver.cpp
  aux_serial_driver.cpp
  audio_driver.cpp
  trainer_driver.cpp
  )

set(SRC
  ${SRC}
  debug.cpp
  )

