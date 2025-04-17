

## ✅ **TouchGFX + SDRAM Framebuffer Setup (STM32F469)**

### 📁 1. **Linker Script Edits**

**Open `LinkerScript.ld`**, and update these sections:

#### 🔹 Add SDRAM memory region:
```ld
MEMORY
{
  RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 320K
  CCMRAM  (xrw) : ORIGIN = 0x10000000, LENGTH = 64K
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
  SDRAM   (xrw) : ORIGIN = 0xC0000000, LENGTH = 8M
}
```

#### 🔹 Add framebuffer section to SECTIONS:
Place this at the **end** of `SECTIONS`:

```ld
.TouchGFX_Framebuffer (NOLOAD) :
{
    . = ALIGN(4);
    KEEP(*(.TouchGFX_Framebuffer))
    . = ALIGN(4);
} >SDRAM
```

---

### 🧠 2. **Manually Force Framebuffer Placement**

In `TouchGFXGeneratedHAL.cpp`, replace the auto-generated buffer definition:

#### ❌ Replace this:
```cpp
LOCATION_PRAGMA_NOLOAD("TouchGFX_Framebuffer")
uint32_t frameBuf[...] LOCATION_ATTRIBUTE_NOLOAD("TouchGFX_Framebuffer");
```

#### ✅ With this:
```cpp
__attribute__((section(".TouchGFX_Framebuffer"), used))
static uint32_t frameBuf[(480 * 800 * 2 + 3) / 4];
```

---

### 🧼 3. **Clean and Rebuild**

After changes:
- Delete the `build/` or `RelWithDebInfo/` directory
- Run CMake and rebuild the project

---

### 🔍 4. **Verify Success in `.map` File**

Look for this in your `.map`:
```txt
.TouchGFX_Framebuffer  0xC0000000   0xBB800   frameBuf
```

---

### 🧠 Optional: Wrap as a Macro (for reuse)

In a header file (e.g., `TouchGFXMemory.hpp`), you can define:

```cpp
#define TOUCHGFX_FRAMEBUFFER_SECTION __attribute__((section(".TouchGFX_Framebuffer"), used))
```

Then use:
```cpp
TOUCHGFX_FRAMEBUFFER_SECTION static uint32_t frameBuf[(480 * 800 * 2 + 3) / 4];
```



# STM32F469I-DISCO TBS

The default IDE is set to STM32CubeIDE, to change IDE open the STM32F469I-DISCO.ioc with STM32CubeMX and select from the supported IDEs (EWARM from version 8.50.9, MDK-ARM, and STM32CubeIDE). Supports flashing of the STM32F469I-DISCO board directly from TouchGFX Designer using GCC and STM32CubeProgrammer.Flashing the board requires STM32CubeProgrammer which can be downloaded from the ST webpage. 

This TBS is configured for 480 x 800 pixels 16bpp screen resolution.  

Performance testing can be done using the GPIO pins designated with the following signals: VSYNC_FREQ  - Pin PA2(D5), RENDER_TIME - Pin PA6(D6), FRAME_RATE  - Pin PA1(D3), MCU_ACTIVE  - Pin PG12(D4)