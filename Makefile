TOOLCHAIN = /root/x-tools/aarch64-rpi3-linux-gnueabi/bin/aarch64-rpi3-linux-gnueabi
DOCKER_IMAGE = toolchain
DOCKER_DIR = /root/build

AARCH = -march=armv6 
CCFLAGS = -O2 -Wall -nostartfiles -ffreestanding $(AARCH)

BLD_DIR = ./build

SOBJ = startup.o
UOBJ = cstartup.o cstubs.o peripheral.o interrupts.o kernel.o gpio.o uart.o timer.o

all: $(SOBJ) $(UOBJ) 
	$(TOOLCHAIN)-gcc $(CCFLAGS) -T linker.ld $(addprefix $(BLD_DIR)/, $(SOBJ)) $(addprefix $(BLD_DIR)/, $(UOBJ)) -o $(BLD_DIR)/kernel.elf
	$(TOOLCHAIN)-objdump -D $(BLD_DIR)/kernel.elf > $(BLD_DIR)/kernel.list
	$(TOOLCHAIN)-objcopy -O binary  $(BLD_DIR)/kernel.elf $(BLD_DIR)/kernel8.img

$(SOBJ):
	$(TOOLCHAIN)-as ./asm/$(basename $@).S -o $(BLD_DIR)/$@

$(UOBJ):
	$(TOOLCHAIN)-gcc -c ./c/$(basename $@).c -o $(BLD_DIR)/$@

clean:
	rm -f $(BLD_DIR)/*

start-toolchain:
	docker run --rm -it -v $(CURDIR):$(DOCKER_DIR) -w $(DOCKER_DIR) $(DOCKER_IMAGE)
