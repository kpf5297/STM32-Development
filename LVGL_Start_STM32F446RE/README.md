
# LVGL STM32F446RE Touchscreen Demo

A simple demo project using the **STM32F446RE Nucleo board** and an **ILI9341-based 320x240 SPI TFT display**, powered by **LVGL v9.2**. Designed with **STM32CubeIDE** on macOS.

## Features

- Uses LVGL 9.2.3-dev
- SPI-based ILI9341 driver (manual implementation)
- Landscape orientation with `ILI9341_SetRotation(3)`
- Demonstrates an LVGL spinner
- Configurable framebuffer size
- No OS / bare-metal

## Pinout (STM32F446RE ➜ ILI9341 SPI)

| ILI9341 Pin | Function         | STM32 Pin         |
|-------------|------------------|--------------------|
| `VCC`       | 3.3V             | 5V               |
| `GND`       | Ground           | GND                |
| `CS`        | Chip Select      | `PA9`              |
| `RESET`     | Reset            | `PC7`              |
| `DC`        | Data/Command     | `PB6`              |
| `SDI/MOSI`  | SPI MOSI         | `PA7` (SPI1_MOSI)  |
| `SCK`       | SPI Clock        | `PA5` (SPI1_SCK)   |
| `SDO/MISO`  | SPI MISO         | `PA6` (SPI1_MISO)  |
| `LED`       | Backlight        | 3.3V or GPIO (PA3) |

> You can configure alternate pins in `main.h` or the `ILI9341.h` driver header.

## Setup

### STM32CubeIDE Project Settings
- Target: `STM32F446RETx`
- Toolchain: `arm-none-eabi-gcc`
- Debug: SWD (via ST-Link)
- Optimization: `-O0` (for debugging)
- Float ABI: `-mfloat-abi=hard`
- FPU: `-mfpu=fpv4-sp-d16`

### ILI9341 Driver Setup
- Minimal 3-wire SPI write driver (8-bit)
- Custom initialization sequence
- Flushes LVGL pixel buffer via `HAL_SPI_Transmit()`
- Use DMA (`ILI9341_DrawBitmapDMA`) if needed

### LVGL Configuration
- Located in `lv_conf.h`
- `LV_COLOR_DEPTH 16` (RGB565)
- `LV_USE_DRAW_SW 1` (software renderer)
- `LV_USE_THEME_DEFAULT 1`
- Theme mode: Light

## Dependencies

- STM32Cube HAL drivers (`STM32F4xx`)
- [LVGL](https://github.com/lvgl/lvgl) (included in `Drivers/lvgl`)
- ILI9341 driver (`Core/Src/ILI9341.c` + `ILI9341.h`)

## Demo Behavior

- Blue background (`#003A57`)
- Centered white spinner (64x64) aligned bottom middle

## How to Build

1. Open with STM32CubeIDE
2. Build project
3. Flash to board
4. Confirm display shows blue screen and spinner

## Directory Structure

LVGL\_Start\_STM32F446RE/
├── Core/
│   ├── Inc/
│   ├── Src/
├── Drivers/
│   ├── CMSIS/
│   ├── STM32F4xx\_HAL\_Driver/
│   └── lvgl/
├── lv\_conf.h
├── makefile
└── README.md

## Notes

- Touchscreen not yet implemented (ILI9341 only)
- SPI speed defaults to CubeMX config (suggest > 20 MHz for performance)
- Optional: add DMA for screen flushing to reduce tearing

---

## Author

Kevin Fox  
[MIT License](LICENSE)

---
