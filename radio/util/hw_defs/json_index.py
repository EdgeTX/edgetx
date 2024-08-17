#
# These methods are used to build helper indexes on JSON structures
#
def build_adc_index(adc_inputs):
    i = 0
    index = {}
    for adc_input in adc_inputs["inputs"]:
        name = adc_input["name"]
        index[name] = i
        i = i + 1

    return index


def append_to_index(d, key, val):
    if key not in d:
        d[key] = []

    d[key].append(val)


def build_adc_gpio_port_index(adc_inputs):
    i = 0
    gpios = {}
    for adc_input in adc_inputs["inputs"]:
        if adc_input["adc"] == "SPI":
            continue

        gpio = adc_input["gpio"]
        if gpio is None:
            i = i + 1
            continue

        if gpio not in gpios:
            gpios[gpio] = []

        pin = {"pin": adc_input["pin"], "idx": i}

        gpios[gpio].append(pin)
        i = i + 1

    return gpios


def build_switch_gpio_port_index(switches):
    gpios = {}
    for switch in switches:
        sw_type = switch["type"]

        if sw_type == "2POS" or sw_type == "FSWITCH":
            gpio = switch.get("gpio")
            pin = switch.get("pin")
            if gpio is not None and pin is not None:
                append_to_index(gpios, gpio, pin)

        elif sw_type == "3POS":
            gpio_high = switch.get("gpio_high")
            pin_high = switch.get("pin_high")
            if gpio_high is not None and pin_high is not None:
                append_to_index(gpios, gpio_high, pin_high)

            gpio_low = switch.get("gpio_low")
            pin_low = switch.get("pin_low")
            if gpio_low is not None and pin_low is not None:
                append_to_index(gpios, gpio_low, pin_low)

    return gpios


def build_trim_gpio_port_index(trims):
    def index_contact(gpios, contact):
        gpio = contact["gpio"]
        pin = contact["pin"]
        if gpio and pin:
            append_to_index(gpios, contact["gpio"], contact["pin"])

    gpios = {}
    for trim in trims:
        dec = trim.get("dec")
        inc = trim.get("inc")
        if dec and inc:
            index_contact(gpios, dec)
            index_contact(gpios, inc)

    return gpios


def build_key_gpio_port_index(keys):
    gpios = {}
    for key in keys:
        append_to_index(gpios, key["gpio"], key["pin"])

    return gpios
