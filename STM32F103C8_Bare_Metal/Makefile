CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
CFLAGS = -mcpu=cortex-m3 -mthumb -Wall -O2 -Iinclude -DSTM32F103xB \
         -Icmsis_device_f1/Include

LDFLAGS = -T STM32F103XB_FLASH.ld -nostartfiles -Wl,--gc-sections --specs=nano.specs -Wl,--undefined=_init


OBJS = src/main.o src/startup_stm32f103xb.o src/system_stm32f1xx.o src/init_stubs.o

main.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/%.o: src/%.s
	$(AS) -mcpu=cortex-m3 -mthumb $< -o $@

clean:
	rm -f src/*.o main.elf
