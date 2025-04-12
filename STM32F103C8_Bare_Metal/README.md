# STM32F103 **Blue Pill** Bare‑Metal Project

A minimal, cross‑platform template that shows how to build, flash, and debug firmware for the **STM32F103C8T6** ("Blue Pill") entirely with free tools—no vendor IDEs required.

---
## What you get
* **Make‑based build** driven by the Arm GNU Toolchain (`arm‑none‑eabi‑gcc`) 14.x
* Ready‑to‑edit **linker script**, **startup code**, and CMSIS **system clock** file
* One‑click flashing with **OpenOCD** or **st‑flash**
* **GDB** debug recipe
* Step‑by‑step setup for **macOS (Apple Silicon & Intel)** **and Windows (10/11)**
* Troubleshooting checklist (e.g. missing `stdint.h`)

---
## 📂 Project Layout
```
STM32_BluePill/
├── include/                  # Device + CMSIS headers
├── src/
│   ├── main.c               # Your application entry
│   ├── system_stm32f1xx.c   # System clock + vector table helper
│   ├── init_stubs.c         # _init / _fini stubs for newlib‑nano
│   └── startup_stm32f103xb.s# Reset & vector table (assembly)
├── CMSIS_5/                 # (optional) full CMSIS repo
├── STM32F103XB_FLASH.ld     # Linker script (128 KiB Flash / 20 KiB SRAM)
├── Makefile                 # Build rules
└── README.md                # You are here
```

---
## 1 · Install Toolchain & Utilities
### macOS (Apple Silicon & Intel)
| What | Fastest path |
|------|--------------|
| **Arm GNU Toolchain 14.x** | `brew install --cask gcc-arm-embedded` |
| **OpenOCD**               | `brew install openocd` |
| **ST‑Link tools (st‑flash)** | `brew install stlink` |
| **Make** (if missing)     | Xcode CLT already includes it |

> **Heads‑up** Homebrew’s formula `arm-none-eabi-gcc` ships **without** newlib headers. Use the **cask** shown above *or* grab the official tarball from Arm’s website and add its `bin/` to your `PATH`.

### Windows 10/11
| What | Option A (recommended) | Option B |
|------|------------------------|----------|
| **Arm GNU Toolchain 14.x** | Download the MSI from Arm Developer → *GNU Toolchain for Arm* → *arm‑gnu‑toolchain‑14.2.rel1‑win64.exe* | `choco install gcc-arm-embedded` |
| **Make** | Tick “Add GNU make” in the MSI or install **MSYS2** (`pacman -S make`) |  |
| **OpenOCD** | `choco install openocd` | Build from source / use STM32CubeProgrammer CLI |
| **ST‑Link tools** | `choco install stlink` |  |

Add the toolchain’s `bin\` directory (e.g. `C:\Program Files\ArmGNUToolchain\14.2.rel1\bin`) to **`PATH`** and open a new *PowerShell* or *Git Bash*.

---
## 2 · Pull in CMSIS / Device Headers
```bash
# macOS / Linux
mkdir -p cmsis_device_f1/Include
cp /wherever/STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include/* \
   cmsis_device_f1/Include/

# or clone CMSIS‑5 for the core headers
git clone https://github.com/ARM-software/CMSIS_5.git
```
Ensure these headers end up in `include/` or the custom include path you set in the **Makefile**:
* `core_cm3.h`
* `stm32f1xx.h`
* `stm32f103xb.h`

The default **Makefile** already contains:
```make
CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O2 \
         -Iinclude -Icmsis_device_f1/Include -DSTM32F103xB
```
Add extra `-I` paths if you place CMSIS elsewhere.

---
## 3 · Build
```bash
make clean   # optional
make         # produces main.elf + .bin/.hex if you add objcopy
```
A successful build prints no errors and leaves `main.elf` in the project root.

---
## 4 · Flash the Board
### OpenOCD (all OSes)
```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
        -c "program main.elf verify reset exit"
```

### st‑flash (ST‑Link CLI)
```bash
arm-none-eabi-objcopy -O binary main.elf main.bin
st-flash write main.bin 0x8000000   # on Blue Pill Flash starts at 0x0800 0000
```

---
## 5 · Debug with GDB
```bash
# Terminal 1 – launch OpenOCD server
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# Terminal 2 – connect with GDB
arm-none-eabi-gdb main.elf
(gdb) target remote :3333
(gdb) monitor reset halt
(gdb) load            # optional – re‑flash via GDB
(gdb) continue
```
Set breakpoints, inspect memory, etc.

---
## 🛠️ Troubleshooting
| Symptom | Fix |
|---------|-----|
| `fatal error: stdint.h: No such file or directory` | Your toolchain lacks **newlib** headers. On macOS use the **cask** (`brew install --cask gcc-arm-embedded`) or Arm’s tarball; on Windows reinstall the MSI with “Add newlib headers”. |
| `Warn : no flash bank found for address 0x00000000` in OpenOCD | You tried to flash a **raw binary** without giving the correct base address. Use `st-flash write main.bin 0x8000000` or let OpenOCD handle `.elf`. |
| `couldn't open main.elf` | Run `make` first or give OpenOCD the absolute path. |
| Blue Pill won’t start after flash | Make sure BOOT0 pin is low, and that your linker script places `Reset_Handler` at 0x0800 0000. |

---
## References
* [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
* [OpenOCD User Guide](http://openocd.org/doc/)
* [ST‑Link Utilities](https://github.com/stlink-org/stlink)
* STM32F1 Reference Manual (RM0008)


