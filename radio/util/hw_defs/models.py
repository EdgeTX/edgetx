from enum import Enum
from typing_extensions import List, Literal, Optional, Tuple, Union

from pydantic import BaseModel
from pydantic_core import from_json


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

InputEnum = StrEnum(
    "InputEnum",
    (
        [f"P{i}" for i in range(1, MAX_POTS)]
        + [f"SL{i}" for i in range(1, MAX_SLIDERS)]
        + [f"EXT{i}" for i in range(1, MAX_EXT)]
        + [f"SW{chr(i)}" for i in range(ord("A"), ord("Z") + 1)]
        + [f"RAW{i}" for i in range(1, MAX_RAW)]
        + ["JSx", "JSy"]
        + ["VBAT", "RTC_BAT"]
    ),
)

InputNameType = Union[StickEnum, InputEnum]

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


class Input(BaseModel):
    name: InputNameType
    type: InputType
    adc: Optional[ADCNameType] = None
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None
    inverted: Optional[bool] = False
    label: Optional[str] = None
    short_label: Optional[str] = None
    default: Optional[FlexType] = None


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


class HardwareDefinition(BaseModel):
    adc_inputs: ADCInputs
    switches: List[Switch]
    keys: List[Key]

    @staticmethod
    def from_json(data: Union[str, bytes, bytearray]) -> "HardwareDefinition":
        return HardwareDefinition.model_validate(from_json(data))
