# The ARM toolchain prefix (32 bit = arm-...-eabi, 64 bit = aarch64-...-gnueabi)
TOOLCHAIN = arm-none-eabi
TOOLCHAIN = /usr/local/gcc-arm-none-eabi-6-2017-q2-update/bin/arm-none-eabi
# TOOLCHAIN = /root/x-tools/armv8-rpi3-linux-gnueabihf/bin/armv8-rpi3-linux-gnueabihf
# TOOLCHAIN = /root/x-tools/aarch64-rpi3-linux-gnueabi/bin/aarch64-rpi3-linux-gnueabi

AARCH = 
CCFLAGS = -nostartfiles -ffreestanding -mfpu=vfp -mcpu=cortex-a53 -ggdb

# AARCH = -march=armv6 
# CCFLAGS = -O2 -Wall -nostartfiles -ffreestanding $(AARCH)

TARGET = kernel8-32
BUILD = build
SOURCE = src

COPY = /Volumes/boot

SOBJ = bootcode.o vectors.o
UOBJ = cstartup.o cstubs.o init.o peripheral.o gpio.o mailbox.o interrupts.o timer.o uart.o multicore.o cache.o
HOBJ = cache.h gpio.h interrupts.h mailbox.h multicore.h peripheral.h timer.h uart.h
KOBJ = kinit.o create.o ctsw.o syscall.o disp.o

HOBJ += kernel/kernel.h kernel/list.h

all: $(BUILD)/$(TARGET).img $(BUILD)/$(TARGET).list

# ELF
$(BUILD)/$(TARGET).elf: $(addprefix $(BUILD)/, $(SOBJ)) $(addprefix $(BUILD)/, $(UOBJ)) $(addprefix $(BUILD)/, $(KOBJ))
	$(TOOLCHAIN)-gcc $(CCFLAGS) -T $(SOURCE)/linker.ld $^ -o $(BUILD)/$(TARGET).elf

# ELF to LIST
$(BUILD)/$(TARGET).list: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objdump -D $(BUILD)/$(TARGET).elf > $(BUILD)/$(TARGET).list

# ELF to IMG
$(BUILD)/$(TARGET).img: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objcopy -O binary $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).img

$(addprefix $(BUILD)/, $(SOBJ)): $(BUILD)/%.o: $(SOURCE)/asm/%.s $(addprefix $(SOURCE)/c/, $(HOBJ))
	$(TOOLCHAIN)-as $(SOURCE)/asm/$(basename $(@F)).s -o $@

$(addprefix $(BUILD)/, $(UOBJ)): $(BUILD)/%.o: $(SOURCE)/c/%.c $(addprefix $(SOURCE)/c/, $(HOBJ))
	$(TOOLCHAIN)-gcc $(CCFLAGS) -c $(SOURCE)/c/$(basename $(@F)).c -o $@

$(addprefix $(BUILD)/, $(KOBJ)): $(BUILD)/%.o: $(SOURCE)/c/kernel/%.c $(addprefix $(SOURCE)/c/, $(HOBJ))
	$(TOOLCHAIN)-gcc $(CCFLAGS) -c $(SOURCE)/c/kernel/$(basename $(@F)).c -o $@

copy: all
	cp $(BUILD)/$(TARGET).img $(COPY)/$(TARGET).img

clean:
	rm -f $(BUILD)/*

# Cross compile the binary using a container with the toolchain already built, when running in this environment, $(TOOLCHAIN) must point to the path within the cointainer
DOCKER_IMAGE = toolchain
DOCKER_BUILD = /root/build
start-toolchain:
	docker run --rm -it -v $(CURDIR):$(DOCKER_BUILD) -v $(COPY):$(COPY) -w $(DOCKER_BUILD) $(DOCKER_IMAGE)

SERIAL_DEVICE = /dev/tty.usbserial-AH069DMB
SERIAL_BAUD_RATE = 115200
serial-screen:
	screen $(SERIAL_DEVICE) $(SERIAL_BAUD_RATE)

# GDB_HOST = localhost
GDB_HOST = docker.for.mac.host.internal

gdb-core-0:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -tui -ex="target remote $(GDB_HOST):3333" -ex=continue

gdb-core-1:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex="target remote $(GDB_HOST):3334" -ex=continue
	
gdb-core-2:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex="target remote $(GDB_HOST):3335" -ex=continue
	
gdb-core-3:
	$(TOOLCHAIN)-gdb $(BUILD)/$(TARGET).elf -ex="target remote $(GDB_HOST):3336" -ex=continue


