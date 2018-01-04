
# GNU ARM Embedded Toolchain
GNU_ARM = /usr/local/gcc-arm-none-eabi-6-2017-q2-update/bin/arm-none-eabi

# The gcc compilers
AARCH = -march=armv6 
CCFLAGS = -O2 -Wall -nostartfiles -ffreestanding $(AARCH)

BLD_DIR = ./build

SOBJ = startup.o
UOBJ = cstartup.o cstubs.o peripheral.o interrupts.o kernel.o gpio.o uart.o timer.o

all: $(SOBJ) $(UOBJ) 
	$(GNU_ARM)-gcc $(CCFLAGS) -T linker.ld $(addprefix $(BLD_DIR)/, $(SOBJ)) $(addprefix $(BLD_DIR)/, $(UOBJ)) -o $(BLD_DIR)/kernel.elf
	$(GNU_ARM)-objdump -D $(BLD_DIR)/kernel.elf >  $(BLD_DIR)/kernel.list
	$(GNU_ARM)-objcopy -O binary  $(BLD_DIR)/kernel.elf  $(BLD_DIR)/kernel8-32.img

$(SOBJ):
	$(GNU_ARM)-as ./asm/$(basename $@).S -o $(BLD_DIR)/$@

$(UOBJ):
	$(GNU_ARM)-gcc -c ./c/$(basename $@).c -o $(BLD_DIR)/$@

run:
	qemu-system-arm -M raspi2 -nographic -kernel  $(BLD_DIR)/kernel.img

clean:
	rm -f $(BLD_DIR)/*
