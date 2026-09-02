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
    # SPI ADC fields
    gpio_pin_sck: Optional[str] = None
    gpio_pin_miso: Optional[str] = None
    gpio_pin_mosi: Optional[str] = None
    gpio_pin_cs: Optional[str] = None


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
        "LUX",
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
    pwm_channel: Optional[int] = None


class FlexInput(BaseModel):
    name: FlexInputEnum
    type: Literal["FLEX"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None
    inverted: Optional[bool] = False
    default: Optional[FlexType] = None
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


class LuxInput(BaseModel):
    name: Literal["LUX"]
    type: Literal["LUX"]
    adc: ADCNameType
    gpio: Optional[str] = None
    pin: Optional[str] = None
    channel: Optional[Union[str, int]] = None

class EXTI(BaseModel):
    irq: str
    priority: int

class Timers(BaseModel):
    cpu_freq: int
    peri1_frequency: int
    peri2_frequency: int
    timer_mult_apb1: int
    timer_mult_apb2: int
    ms_timer: str
    ms_timer_irqn: str
    ms_timer_irqhandler: str
    mixer_scheduler_timer: str
    mixer_scheduler_timer_freq: str
    mixer_scheduler_timer_irqn: str
    mixer_scheduler_timer_irqhandler: str

class Backlight(BaseModel):
    has_backlight_color: Optional[bool] = False
    backlight_gpio: Optional[str] = None
    backlight_timer: Optional[str] = None
    backlight_timer_channel: Optional[str] = None
    backlight_gpio_af: Optional[str] = None
    backlight_timer_freq: Optional[str] = None
    backlight_bdtr: Optional[str] = None

class Display(BaseModel):
    lcd_w: int
    lcd_h: int
    lcd_phys_w: int
    lcd_phys_h: int
    lcd_depth: int
    oled_screen: Optional[bool] = False
    lcd_horizontal_invert: Optional[bool] = False
    lcd_vertical_invert: Optional[bool] = False
    ltdc_irq_prio: Optional[int] = None
    dma_screen_irq_prio: Optional[int] = None

class LEDS(BaseModel):
    led_strip_length: Optional[int] = None
    bling_led_strip_start: Optional[int] = None
    bling_led_strip_length: Optional[int] = None
    cfs_led_strip_start: Optional[int] = None
    cfs_led_strip_length: Optional[int] = None
    cfs_leds_per_switch: Optional[int] = None
    led_strip_gpio: Optional[str] = None
    led_strip_gpio_af: Optional[str] = None
    led_strip_timer: Optional[str] = None
    led_strip_timer_freq: Optional[str] = None
    led_strip_timer_channel: Optional[str] = None
    led_strip_timer_dma: Optional[str] = None
    led_strip_timer_dma_channel: Optional[str] = None
    led_strip_timer_dma_stream: Optional[str] = None
    led_strip_timer_dma_irqn: Optional[str] = None
    led_strip_timer_dma_irqhandler: Optional[str] = None
    led_strip_refresh_period: Optional[int] = None
    status_leds: Optional[bool] = False
    gpio_led_gpio_on: Optional[str] = None
    gpio_led_gpio_off: Optional[str] = None
    led_red_gpio: Optional[str] = None
    led_red2_gpio: Optional[str] = None
    led_green_gpio: Optional[str] = None
    led_blue_gpio: Optional[str] = None

class IMU(BaseModel):
    imu_i2c_bus: Optional[str] = None
    imu_int_gpio: Optional[str] = None
    imu_i2c_address: Optional[str] = None
    exti: Optional[List[EXTI]] = None
    imu_invert_x: Optional[bool] = None
    imu_invert_y: Optional[bool] = None

class RotEnc(BaseModel):
    rotary_encoder_inverted: Optional[bool] = None
    rotary_encoder_gpio: Optional[str] = None
    rotary_encoder_gpio_a: Optional[str] = None
    rotary_encoder_gpio_b: Optional[str] = None
    rotary_encoder_gpio_pin_a: Optional[str] = None
    rotary_encoder_gpio_pin_b: Optional[str] = None
    rotary_encoder_position: Optional[str] = None
    rotary_encoder_exti_line1: Optional[str] = None
    rotary_encoder_exti_line2: Optional[str] = None
    rotary_encoder_exti_port: Optional[str] = None
    rotary_encoder_exti_port_a: Optional[str] = None
    rotary_encoder_exti_port_b: Optional[str] = None
    rotary_encoder_exti_sys_line1: Optional[str] = None
    rotary_encoder_exti_sys_line2: Optional[str] = None
    rotary_encoder_timer: Optional[str] = None
    rotary_encoder_timer_irqn: Optional[str] = None
    rotary_encoder_timer_irqhandler: Optional[str] = None
    exti: Optional[List[EXTI]] = None

class Haptic(BaseModel):
    haptic_pwm: Optional[bool] = None
    haptic_gpio: Optional[str] = None
    haptic_gpio_timer: Optional[str] = None
    haptic_gpio_af: Optional[str] = None
    haptic_timer_output_enable: Optional[str] = None
    haptic_timer_mode: Optional[str] = None
    haptic_timer_compare_value: Optional[str] = None
    haptic_timer: Optional[str] = None
    haptic_timer_freq: Optional[str] = None
    haptic_counter_register: Optional[str] = None
    haptic_ccmr1: Optional[str] = None
    haptic_ccmr2: Optional[str] = None
    haptic_ccer: Optional[str] = None

Input = Union[StickInput, FlexInput, SwitchInput, RawInput, VBatInput, RTCBatInput, LuxInput]


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
    is_cfs: Optional[bool] = False
    cfs_idx: Optional[int] = None
    adc_input: Optional[str] = None
    gpio: Optional[str] = None
    pin: Optional[str] = None
    gpio_high: Optional[str] = None
    pin_high: Optional[str] = None
    gpio_low: Optional[str] = None
    pin_low: Optional[str] = None
    display: Optional[SwitchDisplayType] = None

    def _uses_single_gpio(self: "Switch") -> bool:
        return bool(self.pin)

    def _uses_two_gpios(self: "Switch") -> bool:
        return bool(self.pin_high or self.pin_low)

    def _uses_gpio(self: "Switch") -> bool:
        return bool(self.gpio_high or self.gpio_low or self.gpio)

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
            if str(self.type) not in ["2POS", "FSWITCH"]:
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Single GPIO switch type is either '2POS' or 'FSWITCH'",
                )
            # TODO: check 'default' as well?

        if self._uses_two_gpios():
            if not self.pin_high or not self.pin_low:
                raise PydanticCustomError(
                    "SwitchHardwareError",
                    "Switch missing 'pin_high' or 'pin_low'",
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
    display: Display
    leds: Optional[LEDS] = None
    backlight: Optional[Backlight] = None
    timers: Timers
    imu: Optional[IMU] = None
    rotenc: Optional[RotEnc] = None
    haptic: Optional[Haptic] = None
    key_lock_combo: Optional[List[KeyEnum]] = None

    @model_validator(mode="after")
    def check_key_lock_combo(self: "HardwareDefinition") -> "HardwareDefinition":
        if self.key_lock_combo is None:
            return self
        if len(self.key_lock_combo) != 2:
            raise PydanticCustomError(
                "KeyLockComboError",
                "'key_lock_combo' must list exactly 2 keys",
            )
        if self.key_lock_combo[0] == self.key_lock_combo[1]:
            raise PydanticCustomError(
                "KeyLockComboError",
                "'key_lock_combo' entries must be distinct",
            )
        defined = {k.key for k in self.keys}
        for k in self.key_lock_combo:
            if k not in defined:
                raise PydanticCustomError(
                    "KeyLockComboError",
                    f"'key_lock_combo' references {k} which is not in 'keys'",
                )
        return self

    @staticmethod
    def from_json(data: Union[str, bytes, bytearray]) -> "HardwareDefinition":
        return HardwareDefinition.model_validate(from_json(data))
