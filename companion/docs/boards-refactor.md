# Companion Supplimentary Board Definitions

## Purpose
The radio source includes hardware definition files for all radios.

Companion ingests these files into a Qt resource file hwdefs.qrc during its compilation.

However, there are radio properties these files lack required by Companion to operate.

## Dev Maintenance
The files are manually maintained by devs.

As radios become supported by firmware, their supplimental file(s) need to be created. There must be at least one file per radio containing basic details and not 'hidden' for it to be available in lists.

Unused files are ignored so can be left for reference and/or future use. Note: They will still be ingested into the qrc.

As additional properties are included/removed from the radio hardware source
files they need to be reflected in the supplimentary files.

## File Structure
The file relationships follow a hierarchical structure.

Each node can have one or more parents (except default which is the root node).

There is no theoretical limit to the number of nodes this model can support. It assumes the dev knows what they are doing.

Warning: This does introduce the risk of circular relationships.

Example:
| File | basedOn | Notes |
| ---  | ---     | ---   |
| default.json | <omitted> | process itself
| boxer.json | default | process default->itself |
| tx16smk3.json | default | process default->itself |

 Note: A node process itself after processing each basedOn path

## File Contents
The files are in standard JSON format.

Refer to default.json for a full list of supported properties.

| Name    | Type  | Description | Notes |
| ---     | ---   | ---         | ---   |
| basedOn | array | parent node(s) | default.json should never have this entry <br /> not every node may require a parent if it would duplicate a common structure |
| hidden  | bool  | true means hide from list of available boards |

## Building the Qt Resource for Companion and Simulator
The companion\src\CMakeList.txt includes directives to run a Python script \companion\util\generate_bddefs_qrc.py to construct the qrc file from all .json files found in \companon\src\boards\defs.

## How Board Definitions are Built at Runtime
1. Scan all files defined in the QRC checking for the existence/non-existence of hidden property.
2. For each file with hidden set to true, skip on this pass.
3. For each entry in basedOn navigate up until reaching a file without or an empty basedOn list.
4. In order navigated, retrace each path to its conclusion applying property values.

This method allows for compounding of property values and overriding by children.

Therefore it is critical to order basedOn references from least to most important to achieve the desired compound result.
