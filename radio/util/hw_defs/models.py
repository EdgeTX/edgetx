from enum import Enum
from typing_extensions import List, Literal, Optional, Tuple, Union

from pydantic import BaseModel, model_validator
from pydantic_core import from_json, PydanticCustomError


class StrEnum(str, Enum):
    @staticmethod
    def _generate_next_value_(name, start, count, last_values):
        return name

    def __str__(self) -> str:
        return self.value


ADCNameType = Literal["MAIN", "EXT", "SPI"]


class ADC(BaseModel):
    name: ADCNameType
    adc: str
    sample_time: Optional[str] = None
    dma: Optional[str] = None
    dma_channel: Optional[str] = None
    dma_stream: Optional[str] = None
    dma_stream_irq: Optional[str] = None
    dma_stream_irq_handler: Optional[str] = None


StickEnum = StrEnum(
    "StickEnum",
    (
        # 2 Gimbal radios
        "LH",
        "LV",
        "RV",
        "RH",
        # Surface radios
        "ST",
        "TH",
    ),
)

MAX_POTS = 8
MAX_SLIDERS = 8
MAX_EXT = 16
MAX_RAW = 8

FlexInputEnum = StrEnum(
    "FlexInputEnum",
    (
        [f"P{i}" for i in range(1, MAX_POTS)]
        + [f"SL{i}" for i in range(1, MAX_SLIDERS)]
        + [f"EXT{i}" for i in range(1, MAX_EXT)]
        + ["JSx", "JSy"]
    ),
)

SwitchInputEnum = StrEnum(
    "SwitchInputEnum",
    list([f"SW{chr(i)}" for i in range(ord("A"), ord("Z") + 1)]),
)

RawInputEnum = StrEnum("RawInputEnum", list([f"RAW{i}" for i in range(1, MAX_RAW)]))

InputNameType = Union[StickEnum, FlexInputEnum, SwitchInputEnum, RawInputEnum]

InputType = StrEnum(
    "InputType",
    (
        "STICK",
        "FLEX",
        "SWITCH",
        "VBAT",
        "RTC_BAT",
        "RAW",
    ),
)

FlexType = StrEnum(
    "FlexType",
    (
        "NONE",
        "POT",
        "POT_CENTER",
        "SLIDER",
        "MULTIPOS",
        "AXIS_X",
        "AXIS_Y",
    ),
)


class StickInput(BaseModel):
    name: StickEnum
    type: Literal["STICK"]
    adc: Optional[ADCNameType] = None
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None
    inverted: Optional[bool] = False


class FlexInput(BaseModel):
    name: FlexInputEnum
    type: Literal["FLEX"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None
    inverted: Optional[bool] = False
    default: Optional[FlexType] = FlexType.NONE
    label: Optional[str] = None
    short_label: Optional[str] = None


class SwitchInput(BaseModel):
    name: SwitchInputEnum
    type: Literal["SWITCH"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None
    inverted: Optional[bool] = False


class RawInput(BaseModel):
    name: RawInputEnum
    type: Literal["RAW"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None


class VBatInput(BaseModel):
    name: Literal["VBAT"]
    type: Literal["VBAT"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None


class RTCBatInput(BaseModel):
    name: Literal["RTC_BAT"]
    type: Literal["RTC_BAT"]
    adc: Literal["MAIN", "EXT"]
    channel: str


Input = Union[StickInput, FlexInput, SwitchInput, RawInput, VBatInput, RTCBatInput]


class ADCInputs(BaseModel):
    adcs: List[ADC]
    inputs: List[Input]


# MUST be the same order as 'EnumKeys'
KeyEnum = StrEnum(
    "KeyEnum",
    (
        "KEY_MENU",
        "KEY_EXIT",
        "KEY_ENTER",
        "KEY_PAGEUP",
        "KEY_PAGEDN",
        "KEY_UP",
        "KEY_DOWN",
        "KEY_LEFT",
        "KEY_RIGHT",
        "KEY_PLUS",
        "KEY_MINUS",
        "KEY_MODEL",
        "KEY_TELE",
        "KEY_SYS",
        "KEY_SHIFT",
        "KEY_BIND",
    ),
)


class Key(BaseModel):
    name: str
    label: str
    key: KeyEnum
    active_low: Optional[bool] = False
    gpio: Optional[str] = None
    pin: Optional[str] = None

    @model_validator(mode="after")
    def check_hardware(self: "Key") -> "Key":
        if bool(self.gpio) != bool(self.pin):
            raise PydanticCustomError(
                "KeyHardwareError",
                "Key missing either 'gpio' or 'pin'",
            )

        return self


SwitchHardwareTypeEnum = StrEnum(
    "SwitchTypeEnum",
    (
        "2POS",
        "3POS",
        "ADC",
        "FSWITCH",
    ),
)

SwitchTypeEnum = StrEnum(
    "SwitchTypeEnum",
    (
        "2POS",
        "3POS",
        "TOGGLE",
        "NONE",
    ),
)

SwitchDisplayType = Union[Tuple[int, int], Tuple]


class Switch(BaseModel):
    name: str
    type: SwitchHardwareTypeEnum
    default: Optional[SwitchTypeEnum] = SwitchTypeEnum.NONE
    flags: Optional[int] = 0
    inverted: Optional[bool] = False
    adc_input: Optional[str] = None
    gpio: Optional[str] = None
    pin: Optional[str] = None
    gpio_high: Optional[str] = None
    pin_high: Optional[str] = None
    gpio_low: Optional[str] = None
    pin_low: Optional[str] = None
    display: Optional[SwitchDisplayType] = None

    def _uses_single_gpio(self: "Switch") -> bool:
        return bool(self.gpio or self.pin)

    def _uses_two_gpios(self: "Switch") -> bool:
        return bool(self.gpio_high or self.pin_high or self.gpio_low or self.pin_low)

    def _uses_gpio(self: "Switch") -> bool:
        return self._uses_single_gpio() or self._uses_two_gpios()

    @model_validator(mode="after")
    def check_hardware(self: "Switch") -> "Switch":
        if bool(self.adc_input) == self._uses_gpio():
            if self.adc_input:
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "A switch is either using ADC or it's own GPIO pin(s)",
                )

        if self._uses_gpio() and (self._uses_single_gpio() == self._uses_two_gpios()):
            raise PydanticCustomError(
                "SwitchHardwareError",
                "A switch based on GPIO pin(s) uses either a single or two GPIOs",
            )

        if self._uses_single_gpio():
            if not self.gpio or not self.pin:
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Switch missing either 'gpio' or 'pin'",
                )
            if str(self.type) not in ["2POS", "FSWITCH"]:
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Single GPIO switch type is either '2POS' or 'FSWITCH'",
                )
            # TODO: check 'default' as well?

        if self._uses_two_gpios():
            if (
                not self.gpio_high
                or not self.pin_high
                or not self.gpio_low
                or not self.pin_low
            ):
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Switch missing 'gpio_high' or 'pin_high' or 'gpio_low' or 'pin_low'",
                )
            if str(self.type) != "3POS":
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Dual GPIO switch type is always '3POS",
                )
            # TODO: check 'default' as well?

        return self


class HardwareDefinition(BaseModel):
    adc_inputs: ADCInputs
    switches: List[Switch]
    keys: List[Key]

    @staticmethod
    def from_json(data: Union[str, bytes, bytearray]) -> "HardwareDefinition":
        return HardwareDefinition.model_validate(from_json(data))
