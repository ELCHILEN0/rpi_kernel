
GNU_ARM = arm-none-eabi
BLD_DIR = ./build

SOBJ = startup.o
UOBJ = cstartup.o kernel.o aux.o cstubs.o

all: $(SOBJ) $(UOBJ) 
	$(GNU_ARM)-gcc -nostartfiles -T linker.ld $(addprefix $(BLD_DIR)/, $(SOBJ)) $(addprefix $(BLD_DIR)/, $(UOBJ)) -o $(BLD_DIR)/main.elf
	$(GNU_ARM)-objdump -D $(BLD_DIR)/main.elf >  $(BLD_DIR)/main.list
	$(GNU_ARM)-objcopy -O binary  $(BLD_DIR)/main.elf  $(BLD_DIR)/main.bin

$(SOBJ):
	$(GNU_ARM)-as ./asm/$(basename $@).S -o $(BLD_DIR)/$@

$(UOBJ):
	$(GNU_ARM)-gcc -c ./c/$(basename $@).c -o $(BLD_DIR)/$@

# 	qemu-system-arm -M realview-pb-a8 -nographic -kernel  $(BLD_DIR)/test.bin
run:
	qemu-system-arm -M raspi2 -nographic -kernel  $(BLD_DIR)/main.bin

clean:
	rm -f $(BLD_DIR)/*