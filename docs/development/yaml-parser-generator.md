# YAML Parser Generator

The YAML storage used in EdgeTX since version `2.6` relies on the radio side on a parser
generator that creates a structure in flash memory to be used by the parser to
determine a precise mapping `attribute -> [bit address, bit length]`.

This is done by using `libclang` Python bindings to parse `ModelData` and `RadioData`
as well as related structures to compute precise bit-offsets for each attribute.

## Generating YAML parsers for existing radios

As the Python script used only works for now with certain versions of libclang,
it is highly recommended to use the EdgeTX dev container to generate YAML parsers.

The master script used will generate the parsers for all different radios:
```shell
docker run -it --rm -v $(pwd):/src \
  ghcr.io/edgetx/edgetx-dev \
  /src/tools/generate-yaml.sh
```

It is also possible to generate only for certain radios by using the `FLAVOR` variable:
```shell
docker run -it --rm -v $(pwd):/src \
  -e "FLAVOR=tx16s;x12s;nv14;x7;x9d;x9dp"
  ghcr.io/edgetx/edgetx-dev \
  /src/tools/generate-yaml.sh
```

Please check `tools/generate-yaml.sh` for available target names.

## Generating YAML parsers for new radios

When new radios are integrated, it might be necessary to generate new YAML parser structures
if that new radio does not fit 100% to an existing one in terms of storage.

In this case, a number of preparatory steps shall be taken:
- `radio/src/storage/yaml/CMakeLists.txt`: add the proper file name depending on radio
  and naming conventions (use downcase target name as used in other scripts).
- `radio/src/storage/yaml/yaml_datastructs.cpp`: add the proper generated file depending
  on the radio.

New radios shall not add conversions routines from the old storage format. This means
that the `STORAGE_CONVERSIONS` variable should be set appriately to `221` (YAML format)
in `radio/src/storage/conversions/CMakeLists.txt`.

This will effect that the conversion routines will be excluded from compilation.


## A word of caution on changing `RadioData` and `ModelData` structures

The YAML parser generator does not account for padding bits added automatically by the
compiler. This means that great care should be taken to understand how these padding bits
are added when using bit fields.

For example:
```c++
  NOBACKUP(uint8_t  countryCode:2);
  NOBACKUP(int8_t   pwrOnSpeed:3);
  NOBACKUP(int8_t   pwrOffSpeed:3);
```

These fields define a bit field which fills the supporting `8-bit` type completely. If however
another field would be introduced, it could add padding bits *implicitely*.

```c++
  NOBACKUP(uint8_t  countryCode:2);
  NOBACKUP(int8_t   pwrOnSpeed:3);
  NOBACKUP(int8_t   pwrOffSpeed:3);
  // New field
  NOBACKUP(int8_t   myField:2);
```

The new field `myField` here will introduce a new bit field spread on `8 bits` with only `2`
defined explicitly. The compiler however might introduce `6 padding bits` at the end,
depending on what is defined *after*. If the next attributes are also bit field members, the
padding bits will be added after the next bit field, and so on.

If however a normal attribute is defined next, the padding bits will be added directly after
`myField`, and the parser generator will **not** account for them, so that a shift in the bit
address will be introduced and the complete structure will be broken.

```c++
  NOBACKUP(uint8_t  countryCode:2);
  NOBACKUP(int8_t   pwrOnSpeed:3);
  NOBACKUP(int8_t   pwrOffSpeed:3);
  // New field
  NOBACKUP(int8_t   myField:2);
  // Normal type
  NOBACKUP(int8_t   fullField);
```

In this case, silent padding bits shall be explicitly defined to avoid the issue:
```c++
  NOBACKUP(uint8_t  countryCode:2);
  NOBACKUP(int8_t   pwrOnSpeed:3);
  NOBACKUP(int8_t   pwrOffSpeed:3);
  // New field
  NOBACKUP(int8_t   myField:2);
  NOBACKUP(int8_t   paddingBits:6 SKIP);
  // Normal type
  NOBACKUP(int8_t   fullField);
```

Here we use a special attribute `SKIP` to instruct the parser generator to ignore this field
but still account for it in the bit count.

## Custom fields

When a special behavior shall be implemented for certain attributes, custom functions
can be used to implement that behavior.

This allows for covering the following use cases:
- custom input/output formats
- backward compatibility

### Custom input/output formats

Attributes that require a specific input or output format can use the `CUST()` macro.
This macro takes 2 parameters as follows:
- `read` function: used for reading YAML payload.
- `write` function: used for writing YAML payload.

Example:
```
NOBACKUP(int8_t   beepVolume:4 CUST(r_5pos,w_5pos));
```

Here the parser generator will assume that `r_5pos` and `w_5pos` are 2 existing functions,
with the following prototype:
```c++
bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
uint32_t r_5pos(const YamlNode* node, const char* val, uint8_t val_len);
```

### Full Custom Attributes

Fully Custom Attributes are used when there is no direct relationship between a YAML tag
and a member of the data structure. It allows mainly for implementing complex logic
and backward compatibility. Backward compatibility is reached when using such an attribute
for which only the `read` function is defined, but no `write` function.

Here is an example of such backward compatibility attributes:
```
  CUST_ATTR(auxSerialMode, r_serialMode, nullptr);
  CUST_ATTR(aux2SerialMode, r_serialMode, nullptr);
  NOBACKUP(uint16_t serialPort ARRAY(STORAGE_SERIAL_PORTS,struct_serialConfig,nullptr));
```

Both `auxSerialMode` and `aux2SerialMode` are defined here right in front of the new
`serialPort` array to simplify offset calculations in the function setting the values.

The same macro can also be used to implement `read`/`write` logic, as is used for
module configuration:
```
  CUST_ATTR(subType,r_modSubtype,w_modSubtype);
  ...
  uint8_t subType:4 SKIP;
```

Here the real `subType` attribute is marked `SKIP` so that it is ignored by the parser
generator, and a full custom attribute is used to set this attribute instead
(`CUST_ATTR(subType,r_modSubtype,w_modSubtype)`). This is done this way to allow for different
`enum` types depending on the module type.

The custom functions `r_modSubtype` and `w_modSubtype` select the proper lookup table
depending on another attribute (`type`) in `ModuleData`, which is not possible otherwise.

It allows also at the same time to map the MPM protocol numbers to the real ones (those
as defined in the MPM documentation), whereby EdgeTX use internally different protocol numbers
for historical reasons (special treatment of FrSky protocols).

### Union members

EdgeTX uses `union` extensively. This means however that the parser needs to know which
member to select / output when writing YAML.

This is done by implementing a `selector`, for example as seen in the TelemetrySensor
structure:

```c++
  union {
    NOBACKUP(PACK(struct {
      uint16_t ratio;
      int16_t  offset;
    }) custom);
    NOBACKUP(PACK(struct {
      uint8_t source;
      uint8_t index;
      uint16_t spare SKIP;
    }) cell);
    NOBACKUP(PACK(struct {
      int8_t sources[4];
    }) calc);
    NOBACKUP(PACK(struct {
      uint8_t source;
      uint8_t spare[3] SKIP;
    }) consumption);
    NOBACKUP(PACK(struct {
      uint8_t gps;
      uint8_t alt;
      uint16_t spare SKIP;
    }) dist);
    uint32_t param;
  } NAME(cfg) FUNC(select_sensor_cfg);
```

The `TelemetrySensor` defines a anonymous `union`, which the `NAME(cfg)` macro gives a tag
name, while `FUNC(select_sensor_cfg)` defines the selector function.

The `selector` function is then implemented as follows in `yaml_datastruct_funcs.cpp`:
```c++
uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= offsetof(TelemetrySensor, param);
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->unit < UNIT_FIRST_VIRTUAL) {
    if (sensor->type == TELEM_TYPE_CALCULATED) {
      switch(sensor->formula) {
      case TELEM_FORMULA_CELL: return 1; // cell
      case TELEM_FORMULA_DIST: return 4; // dist
      case TELEM_FORMULA_CONSUMPTION: return 3; // consumption
      case TELEM_FORMULA_TOTALIZE: return 3; // consumption
      default: return 2; // calc
      }
    } else {
      return 0; // custom
    }
  }

  return 5;
}
```

The function starts with computing the structure index to obtain a pointer and basically
returns an index which corresponds to which union member shall be used (as defined,
counting from `0`).

### Forced `enum` type

The parser generator uses member types as defined to determine what to generate.
However, some structure members are in fact `enum` types, whereby declared as `uint8_t` or
similar.

So that these values can be output / read as their string representation, it is necessary
to specify the `enum` type that shall be used as follows:
```c++
uint8_t  func ENUM(LogicalSwitchesFunctions);
```

This will also force the parser generator to process that `enum` type in case it is not used
explictly (by type definition) somewhere else.

This can also be used to force the generation of the lookup table even if the values are not
used directly:
```c++
uint16_t srcRaw:10 ENUM(MixSources) CUST(r_mixSrcRaw,w_mixSrcRaw);
```

This will force the `enum` type to be considered for generation, whereby this specific
attribute will used custom functions.

In this particular case the function can then make use of this lookup table as they see fit.

For example with (reading):
```
yaml_parse_enum(enum_MixSources, val, val_len);
```

Or (writing):
```
str = yaml_output_enum(val, enum_MixSources);
```
