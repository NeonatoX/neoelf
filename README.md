# neoelf

Minimal ELF inspection utility for Linux systems.

neoelf reads only the essential information from ELF files — SONAME, NEEDED libraries, interpreter, machine type, ELF class, RPATH, and RUNPATH — without external dependencies. It is designed to replace `readelf`, `objdump`, or `scanelf` for the routine tasks of package managers and `ldconfig`.

Part of the NeonatoX base system.

---

## Features

- ELF32 and ELF64 support
- Endianness detection
- Reads information from Program Headers (`PT_DYNAMIC`, `PT_INTERP`) instead of Section Headers
- No DWARF, symbols, relocations, or debug info interpretation
- Uses `mmap()` for minimal disk I/O

## Options

| Option | Description |
|--------|-------------|
| (none) | Full human-readable output |
| `--soname` | Print SONAME |
| `--needed` | Print NEEDED libraries |
| `--interp` | Print interpreter path |
| `--machine` | Print machine type |
| `--type` | Print ELF type (EXEC/DYN/REL) |
| `--class` | Print ELF class (ELF32/ELF64) |
| `--rpath` | Print RPATH |
| `--runpath` | Print RUNPATH |
| `--export` | Print machine-readable KEY=VALUE format |
| `--help` | Show usage |

## Examples

```
$ neoelf /bin/ls
File: /bin/ls

Class:
    ELF64

Machine:
    x86_64

Type:
    DYN

Interpreter:
    /lib64/ld-linux-x86-64.so.2

SONAME:
    (none)

NEEDED:
    libcap.so.2
    libc.so.6

RUNPATH:
    (none)

RPATH:
    (none)
```

```
$ neoelf --needed /bin/bash
libreadline.so.8
libhistory.so.8
libncursesw.so.6
libc.so.6
```

```
$ neoelf --soname /lib64/ld-linux-x86-64.so.2
ld-linux-x86-64.so.2
```

```
$ neoelf --export /bin/ls
CLASS=ELF64
TYPE=DYN
MACHINE=x86_64
INTERP=/lib64/ld-linux-x86-64.so.2
SONAME=
NEEDED=libcap.so.2
NEEDED=libc.so.6
RUNPATH=
RPATH=
```

## Build

```bash
make
```

Compiler flags: `-std=c17 -Wall -Wextra -pedantic -Os`

Zero warnings required.

## Requirements

- Linux kernel
- musl or glibc
- No external dependencies (no libelf, no elfutils, no binutils, no pax-utils)
- Only POSIX + `<elf.h>`

## Target size

`< 50 KB` dynamically linked with musl.

Typical stripped binary is around **19 KB**.

## Install

```bash
make install DESTDIR=/path/to/rootfs
```

Installs to `/usr/bin/neoelf`.

## Compatibility

Works correctly with:

- PIE executables
- Normal executables
- Shared libraries
- musl binaries
- glibc binaries

## Source structure

```
src/
    main.c       — CLI argument parsing and main flow
    elf.c        — ELF file opening, validation, Program Header walking
    elf.h        — ELF context and error codes
    dynamic.c    — Dynamic section parsing (DT_* entries)
    dynamic.h    — Dynamic information result struct
    output.c     — Output formatting for all modes
    output.h     — Output mode enumeration
```

## Error codes

| Code | Description |
|------|-------------|
| 0    | Success |
| 1    | Error |

All error messages go to stderr:

```
neoelf: /etc/passwd: not an ELF file
neoelf: /nonexistent: cannot open file
neoelf: truncated.so: truncated ELF file
```

## Design philosophy

- Small codebase
- No external dependencies
- ANSI C (C17)
- No libelf, elfutils, binutils, pax-utils
- Prefer Program Headers over Section Headers
- Minimize disk reads with `mmap()`
- K&R brace style, 4-space indentation
- Variables declared at block start
- Avoid functions longer than ~80 lines
- Clarity over micro-optimizations

## Future extensions

Planned but not yet implemented:

```
--abi
--entry
--osabi
--flags
--segments
--build-id
--hash
--gnu-hash
--notes
```

## License

GNU General Public License v3.0

```
neoelf — Minimal ELF inspection utility.
Copyright (C) 2026  Carlos Sanchez

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

## Author

**Carlos Sanchez**

Part of the NeonatoX project.