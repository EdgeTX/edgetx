# SPDX-License-Identifier: GPL-2.0-only
"""Read-only verification of Mambo application ELF/bin memory boundaries."""
import argparse
import hashlib
from pathlib import Path
import struct
import sys

from elftools.elf.elffile import ELFFile


def inside(start, size, bounds):
    return bounds[0] <= start and start + size <= bounds[1]


def verify(elf_path, binary_path):
    flash = (0x0800C000, 0x080C0000)
    ram = (0x20000000, 0x20015000)
    ccm = (0x10000400, 0x10010000)
    ranges = [flash, ram, ccm]
    with elf_path.open("rb") as handle:
        elf = ELFFile(handle)
        assert elf.elfclass == 32 and elf.little_endian
        assert elf.header["e_machine"] == "EM_ARM"
        file_ranges = []
        for segment in elf.iter_segments():
            if segment["p_type"] != "PT_LOAD":
                continue
            address, size = segment["p_vaddr"], segment["p_memsz"]
            assert any(inside(address, size, bounds) for bounds in ranges), (
                "allocation outside Mambo application regions", hex(address), size)
            if segment["p_filesz"]:
                address, size = segment["p_paddr"], segment["p_filesz"]
                assert inside(address, size, flash), ("load overlaps reserved flash", hex(address), size)
                file_ranges.append((address, address + size))
        assert file_ranges and min(start for start, _ in file_ranges) == flash[0]
        vector = elf.get_section_by_name(".isr_vector")
        assert vector and vector["sh_addr"] == flash[0]
        stack, reset = struct.unpack("<II", vector.data()[:8])
        assert stack == ccm[1] and reset & 1 and inside(reset & ~1, 2, flash)
        assert reset == elf.header["e_entry"]
        symbols = elf.get_section_by_name(".symtab")
        named = {symbol.name: symbol for symbol in symbols.iter_symbols()} if symbols else {}
        assert "mamboKernelSysTickHandler" not in named, "internal RF runtime linked"
        assert not any("StartupBridge" in name for name in named), "internal RF bridge linked"
        for irq, handler in ((10, "EXTI4_IRQHandler"),
                             (17, "DMA1_Stream6_IRQHandler"),
                             (38, "USART2_IRQHandler")):
            symbol = named.get(handler)
            assert symbol and symbol["st_info"]["bind"] == "STB_GLOBAL", (
                "missing strong external CRSF IRQ handler", handler)
            entry, = struct.unpack_from("<I", vector.data(), (irq + 16) * 4)
            assert entry == (symbol["st_value"] | 1), ("wrong CRSF IRQ vector", handler)
            assert entry != (named["Default_Handler"]["st_value"] | 1), (
                "CRSF IRQ points at default trap", handler)
        required = {"_Z14mamboEarlyInitv", "_Z16mamboGimbalsReadv",
                    "_Z24mamboExternalModulePowerb", "_Z27mamboExternalModuleShutdownv"}
        assert required.issubset(named), "missing default Mambo board/bay support"
        assert elf.get_section_by_name(".bootloader") is None or \
            elf.get_section_by_name(".bootloader")["sh_size"] == 0
        binary = binary_path.read_bytes()
        assert binary[:8] == vector.data()[:8]
        assert len(binary) == max(end for _, end in file_ranges) - flash[0]
        # Compare every file-backed load to its exact offset in the raw binary.
        for segment in elf.iter_segments():
            if segment["p_type"] == "PT_LOAD" and segment["p_filesz"]:
                offset = segment["p_paddr"] - flash[0]
                assert binary[offset:offset + segment["p_filesz"]] == segment.data()
    return len(binary), hashlib.sha256(binary).hexdigest()


def main():
    if sys.flags.optimize:
        raise SystemExit("Verification requires Python assertions enabled")
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("binary", type=Path)
    args = parser.parse_args()
    size, digest = verify(args.elf, args.binary)
    print(f"Memory contract passed: {size} application bytes at 0x0800C000.")
    print(f"SHA-256: {digest}")


if __name__ == "__main__":
    main()
