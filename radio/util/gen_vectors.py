#!/usr/bin/env python3

# Copyright (C) 2020 Inria
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

"""Generate the vectors table for a given STM32 CPU line."""

import os
import argparse
import re


VECTORS_FORMAT = """
#include "{cpu_line}.h"
#include "cortex_m_isr.h"

void Default_Handler() {{ default_isr_handler(); }}

/* {cpu_line} specific interrupt vectors */
{isr_lines}

/* CPU specific interrupt vector table */
const isr_t _isr_vector[] ISR_VECTOR = {{
  [0] = (isr_t)&_estack,
{irq_lines}
}};
"""


def parse_cmsis(cmsis_dir, cpu_line):
    """Parse the CMSIS to get the list IRQs."""
    cpu_line_cmsis = os.path.join("{}/{}.h".format(cmsis_dir, cpu_line.lower()))

    with open(cpu_line_cmsis, "rb") as cmsis:
        cmsis_content = cmsis.readlines()

    irq_lines = []
    use_line = False
    for line in cmsis_content:
        try:
            line = line.decode()
        except UnicodeDecodeError:
            # skip line that contains non unicode characters
            continue

        # start filling lines after interrupt Doxygen comment
        if "typedef enum" in line:
            irq_lines = []  # Cleanup any previous content
            use_line = True

        # enum not yet detected: skip
        if not use_line:
            continue

        # Stop at the end of the IRQn_Type enum definition
        if "IRQn_Type" in line:
            break

        # use a regexp to get the available IRQs
        match = re.match(r"[ ]+([a-zA-Z0-9_]+_IRQn)[ ]+= \d+", line)

        # Skip lines that don't match
        if match:
            irq_lines.append(match.group(1).strip())

    isrs = [
        {"irq": irq + " + 16", "func": irq.rsplit("_", 1)[0] + "_IRQHandler"}
        for irq in irq_lines
    ]
    return {"isrs": isrs, "cpu_line": cpu_line}


def generate_vectors(context):
    """Use vector template string to generate the vectors C file."""
    isr_line_format = "WEAK_DEFAULT void {func}();"
    irq_line_format = "  [{irq:<35}] = {func},"

    isr_lines = []
    irq_lines = []

    cortex_m_irqs = [
        "Reset_Handler",
        "NMI_Handler",
        "HardFault_Handler",
        "MemManage_Handler",
        "BusFault_Handler",
        "UsageFault_Handler",
        "0",
        "0",
        "0",
        "0",
        "SVC_Handler",
        "DebugMon_Handler",
        "0",
        "PendSV_Handler",
        "SysTick_Handler",
    ]

    irqn = 1
    for isr in cortex_m_irqs:
        if isr != "0":
            isr_lines.append(isr_line_format.format(func=isr))
        irq_lines.append(irq_line_format.format(irq=irqn, func=isr))
        irqn += 1

    for isr in context["isrs"]:
        isr_line = isr_line_format.format(**isr)
        if isr_line not in isr_lines:
            isr_lines.append(isr_line)
        irq_lines.append(irq_line_format.format(**isr))

    vectors_content = VECTORS_FORMAT.format(
        cpu_line=context["cpu_line"],
        isr_lines="\n".join(isr_lines),
        irq_lines="\n".join(irq_lines),
    )

    print(vectors_content)


def main(args):
    """Main function."""
    context = parse_cmsis(args.cmsis_dir, args.cpu_line)
    generate_vectors(context)


PARSER = argparse.ArgumentParser()
PARSER.add_argument("cmsis_dir", help="CMSIS directory")
PARSER.add_argument("cpu_line", help="STM32 CPU line")

if __name__ == "__main__":
    main(PARSER.parse_args())
