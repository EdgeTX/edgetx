# Companion Firmware Definitions
## Purpose
The hardware definitions from radio source and supplimental board definitions from Companion source provide information about each radio.

There are additional properties required by Companion to replicate the radio firmware business rules for managing radio and model settings.

To that end, additional Companion firmware definition files are utilised.

There is a Companion firmware default.json file inherited by all Companion firmware, one file per radio firmware and shared files.

All values are accessed through firmware and board capabilities.

## Capabilities
One or more firmware values, board values and/or other capabilites can be referenced to produce a capability.

Firmware capabilities can reference board capabilites but not vice versa.

The reason being, firmware references a board but a board does not know the firmware that referenced it as there is no enforced one-to-one relationship.

There can be use cases where multiple firmwares reference the same board.

Warning: capabilities referencing other capabilities can result in circular references and application crash.

## Firmware Definition File Maintenance
The files are manually maintained by devs.

As radios become supported by firmware, their supplimental file(s) need to be created.

There must be at least one file per radio containing basic details and not 'hidden' for it to be available in lists.

Unused files are ignored so can be left for reference and/or future use. Note: They will still be ingested into the qrc.

## File Relationships
The file relationships follow a hierarchical tree structure.

Each node can have one or more parents (except default which is the root node).

There is no theoretical limit to the number of nodes this model can support.

This method allows for overlaying of property values by child nodes.

Therefore, it is critical to order basedOn references from least to most important to achieve the desired overlayed result.

Warning: This does introduce the risk of circular relationships though it should be detected during loading.

## Building the Qt Resource for Companion and Simulator
The companion\src\CMakeList.txt includes directives to run a Python script \companion\util\generate_fwdefs_qrc.py to construct the qrc file from all .json files found in \companon\src\firmware\defs.

## How and When Firmware Definitions are Used at Runtime
### Companion Application Launch
1. Scan firmware definitions directory in the QRC checking the hidden property.
2. For each file with hidden omitted or set to false, register with minimal data. <br /> This ensures Companion has a list of all supported and unsupported firmware whilst minimising application startup time.
3. Fully load the current radio profile firmware definition as below.

Unsupported firmware is not shown in UI lists, however, is available for radio and models and settings conversion.

### Firmware Changed or Referenced
1. If previously fully loaded there is nothing to do.
2. Parse and load default.json
3. Parse the selected firmware json file looking for basedOn key.
4. For each entry in basedOn navigate up until reaching a file without or an empty basedOn list.
5. In order traversed, retrace each path to its conclusion applying property values.
6. Finally apply the referenced firmware definition file.
7. Load the linked board definition as per [Board Definitions](board-definitions.md)

## Definition Files
The files are in standard JSON format.

Refer to firmware default.json for a full list of supported firmware properties.

## Sample Definition Files
The files below are used to generate the composite tx16s mk3 firmware.

File: /companion/src/firmware/defs/default.json
```
{
  "hidden": true,
  "basedOn": [
  ],
  "supported": true,
  "name": "",
  "board": "",
  "gitHubAsset": "",
  "simulator": "",
  "buildOpts": [
  ],
...
}
```

File: /companion/src/firmware/defs/color.json
```
{
  "hidden": true,
  "categories": true,
  "labels": true,
  "modelsList": true,
  "keyShortcuts": 6,
  "modelImage": {
    "filters": "*.bmp|*.jpg|*.png",
    "keepExtn": true,
    "name": 10
  },
  "modelSlots": 0,
  "modelName": 15,
  "quickMenuFavs": 12,
  "sensors": {
    "cnt": 60
  }
}
```

File: /companion/src/firmware/defs/cpuh7.json
```
{
  "hidden": true,
  "globalVars": {
    "cnt": 15
  },
  "sensors": {
    "cnt": 99
  }
}
```

File: /companion/src/firmware/defs/tx16s.json
```
{
  "basedOn": [
    "color"
  ],
  "name": "TX16S / SE / Hall / Masterfire",
  "board": "tx16s",
  "buildOpts": [
    [
      "faichoice",
      "faimode"
    ],
    "nooverridech",
    "noheli",
    "nogvars",
    "lua",
    "flexr9m",
    "bluetooth",
    "nogps"
  ]
}
```

File: /companion/src/firmware/defs/tx16smk3.json
```
{
  "basedOn": [
    "tx16s",
    "cpuh7"
  ],
  "name": "TX16S Mark 3"
}
```

### Load Sequence for tx16smk3.json
default + color + tx16s + cpuh7 + tx16smk3
