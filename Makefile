# --------- Toolchain / Host detection ----------
UNAME_M := $(shell uname -m)

# On Apple Silicon / aarch64 we use the i686 cross toolchain
ifeq ($(UNAME_M),aarch64)
PREFIX := i686-linux-gnu-
else
PREFIX :=
endif

CC      := $(PREFIX)gcc
LD      := $(PREFIX)ld
OBJDUMP := $(PREFIX)objdump
OBJCOPY := $(PREFIX)objcopy
SIZE    := $(PREFIX)size

# --------- Build flags ----------
CFLAGS  := -ffreestanding -mgeneral-regs-only -mno-mmx -m32 -march=i386 -fno-pie -fno-stack-protector -g3 -Wall
ASFLAGS := $(CFLAGS)

# --------- GRUB bits (use system paths) ----------
GRUBMKIMAGE ?= grub-mkimage
BOOTIMG     ?= /usr/lib/grub/i386-pc/boot.img   # Works on Ubuntu when i386-pc modules are installed

# --------- Layout ----------
SDIR := src
ODIR := obj

# C / ASM sources in your repo
CSRC := \
  kernel_main.c \
  rprintf.c \
  interrupt.c \
  keyboard.c \
  page.c

SSRC := \
  multiboot_header.s

# Object file list
COBJ := $(CSRC:%.c=$(ODIR)/%.o)
SOBJ := $(SSRC:%.s=$(ODIR)/%.o)
OBJ  := $(COBJ) $(SOBJ)

# --------- Rules ----------
.PHONY: all bin obj clean run run-gui

all: bin rootfs.img

bin: obj $(OBJ)
	$(LD) -melf_i386 $(OBJ) -Tkernel.ld -o kernel
	$(SIZE) kernel

obj:
	mkdir -p $(ODIR)

# Compile C
$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble .s
$(ODIR)/%.o: $(SDIR)/%.s
	$(CC) $(ASFLAGS) -c -o $@ $<

# --------- Disk image with GRUB ----------
rootfs.img: kernel grub.cfg
	@echo "Creating disk image..."
	dd if=/dev/zero of=$@ bs=1M count=32

	@echo "Creating GRUB core image..."
	$(GRUBMKIMAGE) -p "(hd0,msdos1)/boot/grub" -o grub.img -O i386-pc \
		normal biosdisk multiboot multiboot2 configfile fat part_msdos

	@echo "Writing MBR boot code..."
	dd if=$(BOOTIMG) of=$@ conv=notrunc bs=446 count=1

	@echo "Writing GRUB core to sector 1..."
	dd if=grub.img of=$@ conv=notrunc bs=512 seek=1

	@echo "Partitioning..."
	echo 'start=2048, type=83, bootable' | sfdisk $@

	@echo "Creating FAT16 filesystem in partition..."
	mkfs.vfat --offset 2048 -F16 $@

	@echo "Copying kernel and GRUB config..."
	mcopy -i $@@@1M kernel ::/
	mmd   -i $@@@1M ::/boot
	mmd   -i $@@@1M ::/boot/grub
	mcopy -i $@@@1M grub.cfg ::/boot/grub/grub.cfg

	@echo " -- BUILD COMPLETED SUCCESSFULLY --"

# --------- Run targets ----------
# Headless (recommended): all I/O in your terminal; great for screenshots/logs
run:
	qemu-system-i386 -drive format=raw,file=rootfs.img -nographic -serial mon:stdio

# GUI (may fail in nested VM)
run-gui:
	qemu-system-i386 -drive format=raw,file=rootfs.img

clean:
	rm -f grub.img kernel rootfs.img $(ODIR)/*.o
