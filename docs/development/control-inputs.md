# Control Inputs Refactor

This page documents the concepts and ideas implemented in [#2631](https://github.com/EdgeTX/edgetx/pull/2631).

## General

In `YAML`, only canonical names are used. The UI uses however translated labels
that can be overridden with custom labels.


## Main Controls

Sometimes called sticks, gimbals, wheel and throttle: whatever is used to control
the vehicle.

To be able to implement the stick modes used usually in RC aircrafts, it is
necessary to map the physical controls to canonical labels.

Canonical names shall be used for physical reference:
- calibration data,

Canonical labels are used for logical references:
- inputs
- mixer lines
- etc

Corresponding to the canonical labels, UI labels are used to present translated
names to the user. These UI labels can be overidden by the user by defining custom labels.

### Physical main controls

- Radios with 2 gimbals:
  - `LH`: left gimbal, vertical axis
  - `LV`: left gimbal, horizontal axis
  - `RV`: right gimbal, vertical axis
  - `RH`: right gimbal, horizontal axis

- Surface radios:
  - `WH`: steering wheel
  - `TR`: throttle trigger

### Canonical names

- Radios with 2 gimbals:
  - `Rud`
  - `Ele`
  - `Thr`
  - `Ail`

- Surface radios:
  - `Whl`
  - `Thr`


## Additional controls

Additional analog controls have canonical names equal to the canonical labels.

On top of these, labels can be defined to be used in the UI and thus correspond
to the real labels printed on the radio case on in the user manual. These labels can be
overidden by the user by defining custom labels.

### Potentiometers, sliders, joystick

Canonical / physical names:
- `P1`, `P2`, etc
- `SL1`, `SL2`, etc
- `JSx` and `JSy`

### Switches

Canonical names:
- `SA` to `SZ`


## APIs

We use different APIs for the different types of names and labels.

All APIs used the same types as follows:

```
enum {
  ADC_INPUT_MAIN,
  ADC_INPUT_POT,
  ADC_INPUT_AXIS,
  ADC_INPUT_VBAT,
  ADC_INPUT_RTC_BAT
};
```

### Physical names

Physical names are manipulated by using the following functions:

```
const char* analogGetPhysicalName(uint8_t type, uint8_t idx);
int analogLookupPhysicalIdx(uint8_t type, const char* name, size_t len);
```

### Canonical names

Canonical names are retrieved by using the following function:

```
const char* analogGetCanonicalName(uint8_t type, uint8_t idx);
int analogLookupCanonicalIdx(uint8_t type, const char* name, size_t len);
```

### UI labels

UI labels are retrieved using the following functions:

```
# Retrievs the name of the input (custom or canonical)
const char* getAnalogLabel(uint8_t type, uint8_t idx);

# Custom labels
void analogSetCustomLabel(uint8_t type, uint8_t idx, const char* str, size_t len);
const char* analogGetCustomLabel(uint8_t type, uint8_t idx);
bool analogHasCustomLabel(uint8_t type, uint8_t idx);
```
