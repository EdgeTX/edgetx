# Companion Firmware Definitions

## Purpose
Whilst the board definitions from radio source and Companion supplimental provide information abount the hardware, there are additional properties required for Companion to understand the firmware capabilities.

## Dev Maintenance
The files are manually maintained by devs.

As radios become supported by firmware, their supplimental file(s) need to be created. There must be at least one file per radio containing basic details and not 'hidden' for it to be available in lists.

Unused files are ignored so can be left for reference and/or future use. Note: They will still be ingested into the qrc.

## File Structure
The file relationships follow a hierarchical structure.

Each node can have one or more parents (except default which is the root node).

There is no theoretical limit to the number of nodes this model can support. It assumes the dev knows what they are doing.

Warning: This does introduce the risk of circular relationships.

## File Contents
The files are in standard JSON format.

Refer to default.json for a full list of supported properties.

| Name    | Type  | Description | Notes |
| ---     | ---   | ---         | ---   |
| basedOn | array | parent node(s) | default.json should never have this entry <br /> not every node may require a parent if it would duplicate a common structure |
| hidden  | bool  | use for interim nodes | true means hide from list of available firmwares |
| options | array | options and/or option groups | group options are mutual exclusive |

#### Example Files
| File | Property | Value |
| ---  | ---      | ---   |
| default  | hidden     | true |
|          | board      | unknown |
|          | gvars      | 8 |
|          | categories | true |
| color    | basedOn    | default |
|          | hidden     | true |
|          | categories | true |
| bw       | basedOn    | default |
|          | hidden     | true |
|          | categories | false |
| h7       | hidden     | true |
|          | gvars      | 15 |
|          | options    | "opt1" |
| tx16smk3 | basedOn    | color, h7 |
|          | id         | tx16smk3 |
|          | name       | TX16S MK 3 |
|          | board      | tx16smk3 |
|          | options    | "opt2", [ "opt3", "opt4" ]
| boxer    | basedOn    | bw |
|          | id         | boxer |
|          | name       | Boxer |
|          | board      | boxer |

## Building the Qt Resource for Companion and Simulator
The companion\src\CMakeList.txt includes directives to run a Python script \companion\util\generate_fwdefs_qrc.py to construct the qrc file from all .json files found in \companon\src\firmware\defs.

## How Board Definitions are Built at Runtime
1. Scan all files defined in the QRC checking for the existence/non-existence of hidden property.
2. For each file with hidden set to true, skip on this pass.
3. For each entry in basedOn navigate up until reaching a file without or an empty basedOn list.
4. In order navigated, retrace each path to its conclusion applying property values.

This method allows for overlaying of property values by child nodes.

Therefore, it is critical to order basedOn references from least to most important to achieve the desired overlayed result.

#### Example for tx16smk3.json
| File | basedOn | Process |
| ---  | ---     | ---   |
| tx16smk3.json | color, h7 | default->color->h7->itself |

 Note: A node processes itself after processing each basedOn path to ensure its settings are not trampled on.

End results based on example files above

| Property   | Value |
| ---        | ---   |
| board      | tx16smk3 |
| categories | true |
| color      | true |
| gvars      | 15 |
| id         | tx16smk3 |
| name       | TX16S MK 3 |
| options    | "opt1", "opt2", [ "opt3", "opt4" ]

| Property   | Value |
| ---        | ---   |
| board      | boxer |
| categories | false |
| color      | false |
| gvars      | 8 |
| id         | boxer |
| name       | Boxer |

