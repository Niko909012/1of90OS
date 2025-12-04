CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-builtin -fno-stack-protector -O0 -g -Wall
LDFLAGS = -m elf_i386 -T linker.ld

OBJS = boot.o main.o

all: iso/1of90os.iso

boot.o: kernel/boot.s
	$(CC) $(CFLAGS) -m32 -c $< -o $@

main.o: kernel/main.c
	$(CC) $(CFLAGS) -m32 -c $< -o $@

kernel.bin: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJS)

iso/boot/grub/grub.cfg: grub.cfg
	mkdir -p iso/boot/grub
	cp grub.cfg iso/boot/grub/

iso/1of90os.iso: kernel.bin iso/boot/grub/grub.cfg
	mkdir -p iso/boot
	cp kernel.bin iso/boot/

	# Use helper script to create ISO (handles grub-mkrescue/xorriso quirks)
	./scripts/mkiso.sh $@ iso

clean:
	rm -f *.o *.bin
	rm -rf iso

.PHONY: all clean
