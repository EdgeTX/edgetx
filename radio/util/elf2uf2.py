#!/usr/bin/python
from __future__ import annotations
import argparse
import struct
import sys

from typing import BinaryIO, Literal, Optional
from elftools.elf.elffile import ELFFile

DATA_BLOCK_SIZE = 256

UF2_MAGIC_START = b"UF2\n\x57\x51\x5d\x9e"
UF2_MAGIC_END = b"\x30\x6f\xb1\n"

UF2_MAX_BLOCK_SIZE = 512

# extensions
UF2_VERSION_TAG = 0x9FC7BC
UF2_DEVICE_TAG = 0x650D9D
UF2_REBOOT_TAG = 0xE60835


def pad32(b: bytes) -> bytes:
    rem = len(b) % 4
    if rem == 0:
        return b

    return b + (b"\x00" * (4 - rem))


class UF2Extension:
    def __init__(self, tag: int, payload: bytes):
        self.tag = tag
        self.payload = payload

    def __str__(self):
        return f"0x{self.tag:06X}: {self.payload}"

    def _len(self):
        return 4 + len(self.payload)

    def to_bytes(self):
        return (
            self._len().to_bytes(1, "little")
            + self.tag.to_bytes(3, "little")
            + pad32(self.payload)
        )


class UF2Writer:
    def __init__(
        self,
        fstream: BinaryIO,
        *,
        block_size=DATA_BLOCK_SIZE,
        family=0,
        device="",
        version="",
    ):
        self.out = fstream
        self.block_size = block_size
        self.family = family
        self.blockno = 0

        self.extensions = []
        if device is not None and len(device):
            self.extensions.append(UF2Extension(UF2_DEVICE_TAG, device.encode("utf-8")))

        if version is not None and len(version):
            self.extensions.append(
                UF2Extension(UF2_VERSION_TAG, version.encode("utf-8"))
            )

    def _write_block(
        self,
        addr: int,
        payload: bytes,
        nblocks: int,
        /,
        flags=0,
        extensions: list[UF2Extension] = [],
    ):
        if self.family:
            flags |= 0x2000

        merged_extensions = extensions + self.extensions
        if len(merged_extensions) > 0:
            flags |= 0x8000

        hdr = UF2_MAGIC_START + struct.pack(
            b"<IIIIII", flags, addr, len(payload), self.blockno, nblocks, self.family
        )
        size = len(hdr) + len(payload)

        self.out.write(hdr)
        self.out.write(payload)

        for ext in merged_extensions:
            ext_bytes = ext.to_bytes()
            self.out.write(ext_bytes)
            size += len(ext_bytes)

        fill = UF2_MAX_BLOCK_SIZE - size - 4
        self.out.write(b"\00" * fill)
        self.out.write(UF2_MAGIC_END)
        self.blockno += 1


class ELFReader:
    def __init__(self, fstream):
        self.elf = ELFFile(fstream)
        self.segments = self._get_merged_elf_segments()

    def _get_merged_elf_segments(self):
        def iter_segments():
            for i in range(self.elf.num_segments()):
                segment = self.elf.get_segment(i)
                type = segment["p_type"]
                addr = segment["p_paddr"]
                filesz = segment["p_filesz"]
                if type == "PT_LOAD" and filesz > 0:
                    yield (i, addr, filesz)

        def is_mergeable(last_addr, addr):
            if last_addr is None:
                return False
            diff = addr - last_addr
            return diff >= 0 and diff < 256

        segments = []
        last_addr = None
        for i, addr, filesz in iter_segments():
            if is_mergeable(last_addr, addr):
                saddr, size, chunks = segments[-1]
                fill = addr - last_addr
                if fill > 0:
                    chunks.append((-1, fill))
                    size += fill
                chunks.append((i, filesz))
                size += filesz
                segments[-1] = (saddr, size, chunks)
                last_addr = saddr + size
            else:
                segments.append((addr, filesz, [(i, filesz)]))
                last_addr = addr + filesz

        return segments

    def iter_blocks(self, chunks, *, block_size=DATA_BLOCK_SIZE):
        buffer = bytearray()
        for i, size in chunks:
            # fill buffer first
            if i >= 0:
                buffer += self.elf.get_segment(i).data()
            else:
                buffer += b"\x00" * size
            # then drain it
            while len(buffer) >= block_size:
                yield buffer[:block_size]
                buffer = buffer[block_size:]

        # and finally drain it completely
        while len(buffer) > 0:
            yield buffer[:block_size]
            buffer = buffer[block_size:]

    def get_total_blocks(self, /, block_size=DATA_BLOCK_SIZE) -> int:
        blocks = 0
        for _, size, _ in self.segments:
            blocks += (size + block_size - 1) // block_size
        return blocks

    def get_total_size(self) -> int:
        total_size = 0
        for _, size, _ in self.segments:
            total_size += size
        return total_size

    def get_section(self, name: str) -> tuple[int, int]:
        section = self.elf.get_section_by_name(name)
        if section is not None:
            return (section["sh_addr"], section["sh_size"])
        return (0, 0)


class UF2InsertBlock:
    def __init__(
        self,
        insert_addr: int,
        addr: int,
        payload: bytes,
        *,
        extensions: list[UF2Extension] = [],
        flags=0,
    ):
        self.insert_addr = insert_addr
        self.addr = addr
        self.payload = payload
        self.extensions = extensions
        self.flags = flags


def write_uf2(
    reader: ELFReader, uf2: UF2Writer, insert_block: Optional[UF2InsertBlock] = None
):
    nblocks = reader.get_total_blocks() + (1 if insert_block else 0)
    for addr, _, chunks in reader.segments:
        block_addr = addr
        for block in reader.iter_blocks(chunks):
            if insert_block and insert_block.insert_addr <= block_addr:
                uf2._write_block(
                    insert_block.addr,
                    insert_block.payload,
                    nblocks,
                    flags=insert_block.flags,
                    extensions=insert_block.extensions,
                )
                insert_block = None

            if len(block) < DATA_BLOCK_SIZE:
                block = block + b"\xff" * (DATA_BLOCK_SIZE - len(block))

            uf2._write_block(block_addr, block, nblocks)
            block_addr += len(block)


def elf_to_uf2(args: argparse.Namespace) -> None:
    reader = ELFReader(args.input)

    total_blocks = reader.get_total_blocks()
    print(f"Total blocks: {total_blocks}")

    total_size = reader.get_total_size()
    print(f"Total size: {total_size}")

    insert_block = None
    if args.reboot is not None:
        if args.reboot == "auto":
            addr, _ = reader.get_section(".reboot_buffer")
            start, bl_size = reader.get_section(".bootloader")
            insert = start + bl_size
        else:
            insert, addr, start = args.reboot

        print(f"Insert reboot block @{insert:08X} ({addr:08X}/{start:08X})")
        insert_block = UF2InsertBlock(
            insert,
            addr,
            b"BDFU",
            extensions=[UF2Extension(UF2_REBOOT_TAG, start.to_bytes(4, "little"))],
            flags=1,
        )

    uf2 = UF2Writer(
        args.output, family=args.family, device=args.device, version=args.version
    )
    write_uf2(reader, uf2, insert_block)
    print(f"Blocks written: {uf2.blockno}")


def parse_reboot_arg(arg: str) -> tuple[int, int, int] | Literal["auto"]:
    if arg == "auto":
        return arg

    addresses = arg.split(":")
    if len(addresses) != 2:
        raise argparse.ArgumentTypeError("invalid addresses")

    try:
        insert = int(addresses[0], 0)
        addr = int(addresses[1], 0)
        start = int(addresses[2], 0)
        return (insert, addr, start)
    except ValueError:
        raise argparse.ArgumentTypeError("invalid addresses")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--family", type=lambda x: int(x, 0), default=0, help="UF2 family ID"
    )
    parser.add_argument("--device", default="", help="UF2 device description")
    parser.add_argument("--version", default="", help="UF2 firmware version")
    parser.add_argument(
        "--reboot",
        type=parse_reboot_arg,
        metavar="{INSERT:ADDRESS:START | auto}",
        help="add reboot block",
    )
    parser.add_argument(
        "-i",
        "--input",
        type=argparse.FileType("rb"),
        required=True,
        help="input ELF file",
    )
    parser.add_argument(
        "output",
        nargs="?",
        help="output UF2 file",
        type=argparse.FileType("wb"),
        default="output.uf2",
    )
    return parser.parse_args()


def main() -> None:
    try:
        args = parse_args()
    except Exception as e:
        print(e)
        sys.exit(-1)

    try:
        elf_to_uf2(args)
    except Exception as e:
        print(e)
        sys.exit(-1)
    finally:
        args.input.close()
        args.output.close()


if __name__ == "__main__":
    main()
