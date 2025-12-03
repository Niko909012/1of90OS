CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -O0 -g -Wall
LDFLAGS = -m elf_i386 -T linker.ld

OBJS = boot.o main.o pci.o

STUB_OBJS = stub.o

all: iso/1of90os.iso

boot.o: kernel/boot.s
	$(CC) $(CFLAGS) -m32 -c $< -o $@

main.o: kernel/main.c
	$(CC) $(CFLAGS) -m32 -c $< -o $@

pci.o: kernel/pci.c
	$(CC) $(CFLAGS) -m32 -c $< -o $@

kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJS)

stub.bin: kernel/stub.s
	$(CC) $(CFLAGS) -m32 -c $< -o stub.o
	$(LD) -m elf_i386 -T linker.ld -o stub.bin stub.o

kernel.raw: kernel.bin
	# raw binary of kernel (module for the stub)
	objcopy -O binary kernel.bin kernel.raw || true

iso/boot/grub/grub.cfg: grub.cfg
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/

iso/1of90os.iso: stub.bin kernel.raw iso/boot/grub/grub.cfg
	mkdir -p iso/boot
	cp stub.bin iso/boot/
	cp kernel.raw iso/boot/

	# Use helper script to create ISO (handles grub-mkrescue/xorriso quirks)
	./scripts/mkiso.sh $@ iso

clean:
	rm -f *.o *.bin
	rm -rf iso

.PHONY: all clean
