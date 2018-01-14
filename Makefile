# The ARM toolchain prefix
# TOOLCHAIN = arm-none-eabi
TOOLCHAIN = /root/x-tools/aarch64-rpi3-linux-gnueabi/bin/aarch64-rpi3-linux-gnueabi

AARCH = 
CCFLAGS = -nostartfiles -ffreestanding

# AARCH = -march=armv6 
# CCFLAGS = -O2 -Wall -nostartfiles -ffreestanding $(AARCH)

TARGET = kernel
BUILD = build
SOURCE = src

SOBJ = startup.o
UOBJ = cstartup.o kernel.o peripheral.o gpio.o mailbox.o

# SOBJ = startup.o
# UOBJ = cstartup.o cstubs.o peripheral.o interrupts.o kernel.o gpio.o uart.o timer.o

all: $(BUILD)/$(TARGET).img $(BUILD)/$(TARGET).list

# ELF
$(BUILD)/$(TARGET).elf: $(SOBJ) $(UOBJ)
	$(TOOLCHAIN)-gcc $(CCFLAGS) -T $(SOURCE)/linker.ld $(addprefix $(BUILD)/, $(SOBJ)) $(addprefix $(BUILD)/, $(UOBJ)) -o $(BUILD)/$(TARGET).elf

# ELF to LIST
$(BUILD)/$(TARGET).list: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objdump -D $(BUILD)/$(TARGET).elf > $(BUILD)/$(TARGET).list

# ELF to IMG
$(BUILD)/$(TARGET).img: $(BUILD)/$(TARGET).elf
	$(TOOLCHAIN)-objcopy -O binary $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).img

$(SOBJ): %.o: $(SOURCE)/asm/%.s
	$(TOOLCHAIN)-as $(SOURCE)/asm/$(basename $@).s -o $(BUILD)/$@

$(UOBJ): %.o: $(SOURCE)/c/%.c
	$(TOOLCHAIN)-gcc -c $(SOURCE)/c/$(basename $@).c -o $(BUILD)/$@

copy: all
	cp $(BUILD)/$(TARGET).img /Volumes/boot/$(TARGET).img

clean:
	rm -f $(BUILD)/*

# Cross compile the binary using a container with the toolchain already built, when running in this environment, $(TOOLCHAIN) must point to the path within the cointainer
DOCKER_IMAGE = toolchain
DOCKER_BUILD = /root/build
start-toolchain:
	docker run --rm -it -v $(CURDIR):$(DOCKER_BUILD) -w $(DOCKER_BUILD) $(DOCKER_IMAGE)
