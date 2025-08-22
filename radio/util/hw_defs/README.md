# Hardware Definitions

The Python scripts in this directory are responsible for generating different
include files used to describe the hardware to the software.

## Data model

The data model supported is described `models.py`. A script
(`json_validator.py`) is available that validates the JSON files based on this
model.

## Code generator

The various include files are generated based on Jinja2 templates and `generator.py`
provided with a JSON target definition.

`legacy_names.py` contains some hard-coded definitions providing additional
information related to analog inputs (by target; mostly names and labels).
