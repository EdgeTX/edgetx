# Companion Radio Management
## Objective
The objective behind the design is to:
- parameterise radios utilising hardware, firmware and board definition files;
- drive business rules through capabilities;
- eliminination or at worse minimise code changes to support new radios.

Capabilities are maintained that reference one or mode properties from the definition files and optionally other capabilities.

The business rules are applied by code references to capabilities to manage data and UI.

DO NOT scatter specific radio references throughout the code. Capabilities are to be used for this purpose.

## Add a Supported Radio
Steps:
1. Create board and firmware definition files.
2. If necessary, add new capabilities.
3. Add code to reference the new capabilities.

## Remove Radio Support
Edit the radio's firmware and board definitons, adding key "supported" with the value "false"

This will remove the radio from UI pick lists but still make it available for raio and model settings conversion to a supported radio.

## Housekeeping
Periodically, most likely at major releases, remove files and code required for data conversion.

### Firmware and Board Definitions
Remove unused firmware and board definitions.

### Capabilities
Remove unused capabilties from the list and throughout the code.

### Radio and Model Settings Conversion
Remove radio references from unsupported conversions.
