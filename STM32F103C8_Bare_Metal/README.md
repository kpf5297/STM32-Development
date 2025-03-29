# STM32F103 Blue Pill Bare-Metal Project

This project demonstrates how to set up, compile, and flash a bare-metal firmware for the STM32F103 Blue Pill using `arm-none-eabi-gcc` and OpenOCD.

## 📌 Prerequisites
Ensure you have the following tools installed on macOS:

```sh
brew install arm-none-eabi-gcc openocd stlink
```

If you haven't already, install `stlink` for flashing:

```sh
brew install stlink
```

## 📂 Project Structure
```
STM32_Project/
├── include/                  # Header files (CMSIS, device-specific headers)
├── src/                      # Source files
│   ├── main.c                # Main program
│   ├── system_stm32f1xx.c    # System clock configuration
│   ├── init_stubs.c          # Stub functions (_init fix)
│   ├── startup_stm32f103xb.s # Startup assembly file
├── CMSIS_5/                  # CMSIS 5 framework
├── Makefile                  # Build system
├── STM32F103XB_FLASH.ld      # Linker script
└── README.md                 # This file
```

## 🔧 Setting Up
1. Clone or copy the necessary CMSIS and device files:
   ```sh
   mkdir -p cmsis_device_f1/Include
   cp /path/to/cmsis/device_headers/* cmsis_device_f1/Include/
   ```
   Ensure that `stm32f1xx.h`, `core_cm3.h`, and other required headers are in the `include/` folder.

2. Clone CMSIS_5 if not already included:
   ```sh
   git clone https://github.com/ARM-software/CMSIS_5.git
   ```
   Ensure your Makefile includes the correct paths for CMSIS_5:
   ```make
   CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O2 -Iinclude -DSTM32F103xB -Icmsis_device_f1/Include -ICMSIS_5/CMSIS/Core/Include
   ```

## 🏗️ Building the Project
To build the firmware, run:
```sh
make clean
make
```
This generates the `main.elf` binary.

## 📤 Flashing the Firmware
Once built, you can flash it using OpenOCD:

```sh
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program main.elf verify reset exit"
```

Alternatively, if `st-flash` is installed:
```sh
arm-none-eabi-objcopy -O binary main.elf main.bin
st-flash write main.bin 0x8000000
```

## 🐞 Debugging with GDB
To debug using GDB:

1. Start OpenOCD:
   ```sh
   openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
   ```

2. In another terminal, run:
   ```sh
   arm-none-eabi-gdb main.elf
   ```

3. Connect to OpenOCD:
   ```gdb
   target remote :3333
   monitor reset halt
   load
   continue
   ```

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program main.elf verify reset exit"

```
