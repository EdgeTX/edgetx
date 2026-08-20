# Companion Board Definitions
## Purpose
The hardware definitions from radio source provide information about each radio.

There are additional properties required by Companion to replicate the radio firmware business rules for managing radio and model settings.

To that end, additional Companion board definition files are utilised.

There is a Companion board default.json file inherited by all Companion boards, one file per radio firmware and shared files.

All values are accessed through firmware and board capabilities.

## Capabilities
One or more board values and/or other capabilites can be referenced to produce a capability.

Firmware capabilities can reference board capabilites but not vice versa.

The reason being, firmware references a board but a board does not know the firmware that referenced it as there is no enforced one-to-one relationship.

There can be use cases where multiple firmwares reference the same board.

Warning: capabilities referencing other capabilities can result in circular references and application crash.

## Firmware Definition File Maintenance
The files are manually maintained by devs.

As radios become supported their Companion board file(s) need to be created.

Unused files are ignored so can be left for reference and/or future use. Note: They will still be ingested into the qrc.

## File Relationships
The file relationships follow a hierarchical tree structure.

Each node can have one or more parents (except default which is the root node).

There is no theoretical limit to the number of nodes this model can support.

This method allows for overlaying of property values by child nodes.

Therefore, it is critical to order basedOn references from least to most important to achieve the desired overlayed result.

Warning: This does introduce the risk of circular relationships though it should be detected during loading.

## Building the Qt Resource for Companion and Simulator
The companion\src\CMakeList.txt includes directives to run a Python script \companion\util\generate_bddefs_qrc.py to construct the qrc file from all .json files found in \companon\src\boards\defs.

## How and When Firmware Definitions are Used at Runtime
### Companion Application Launch

1. Scan board definitions directory in the QRC checking the hidden property.
2. For each file with hidden omitted or set to false, register with minimal data. <br /> This ensures Companion has a list of all supported and unsupported boards whilst minimising application startup time.

Unsupported boards are available for radio and models and settings conversion.

### Firmware Referenced Triggering Associated Board Load or Board Directly Referenced
When firmware is referenced it will trigger the associated board load to ensure valid capabilities are available.

There are use cases where boards are directly referenced e.g. Radio and model setting conversions from and to boards.

1. If previously fully loaded there is nothing to do.
2. Parse and load default.json
3. Parse the selected board json file looking for hwdefn key.
4. Load the hwdefn file.
5. Parse the selected board json file looking for basedOn key.
6. For each entry in basedOn navigate up until reaching a file with and empty basedOn list.
7. In reverse order to traversal, retrace each path to its conclusion applying property values.
8. Finally apply the referenced board definition file.

## Definition Files
The files are in standard JSON format.

Refer to boards default.json for a full list of supported board properties.

## Sample Definition Files
The file below are used to generate the composite tx16s mk3 board.

File: /radio/src/boards/hw_defs/tx16smk3.json
```
{
  "identity": {
    "manufacturer": "RadioMaster",
    "model": "TX16S MK3",
    "usb_manufacturer": "RM",
    "usb_product": "TX16SMK3"
  },
  "adc_inputs": {
...
  }
}
```

File: /companion/src/boards/defs/default.json
```
{
  "hidden": true,
  "name": "Default",
  "supported": true,
  "hwdefn": "",
...
}
```

File: /companion/src/boards/defs/color.json
```
{
  "hidden": true,
  "backlight": {
    "offBrightness": 20
  }
}
```

File: /companion/src/boards/defs/inputs_imu.json
```
{
  "comments": "usually missing from hwdef",
  "hidden": true,
  "adc_inputs": {
    "inputs": [
      {
        "name": "TILT_X",
        "type": "FLEX",
        "label": "Tltx",
        "short_label": "X",
        "default": "AXIS_X"
      },
      {
        "name": "TILT_Y",
        "type": "FLEX",
        "label": "Tlty",
        "short_label": "Y",
        "default": "AXIS_Y"
      }
    ]
  }
}
```

File: /companion/src/boards/defs/switches_flex.json
```
{
  "comments": "MUST be listed in basedOn BEFORE voice_switches",
  "hidden": true,
  "switches": [
    {
    "name": "FL1"
    },
    {
    "name": "FL2"
    }
  ]
}
```

File: /companion/src/boards/defs/tx16s.json
```
{
  "basedOn": [
    "switches_flex"
  ],
  "name": "TX16S"
}
```

File: /companion/src/boards/defs/tx16smk3.json
```
{
  "basedOn": [
    "tx16s",
    "inputs_imu"
  ],
  "name": "TX16S MK 3",
  "chargeWhileOn": true,
  "vcpSerialMode": true
}
```

### Load Sequence for tx16smk3.json
default + hwdefn + color + switches_flex + tx16s + inputs_imu + tx16smk3
