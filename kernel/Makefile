NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c" -o -name "*.S")
INC_DIR        := include/ framework/
export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH := x86-qemu
endif

PREBUILD := git initrd
include $(AM_HOME)/Makefile.app
include ../Makefile.lab

.PHONY: initrd

initrd:
	@seq 1 8192 > build/ramdisk.img

QEMU_FLAGS := -serial stdio -machine accel=kvm:tcg -drive format=raw,file=$(BINARY)

run1: image
	qemu-system-i386 -smp 1 $(QEMU_FLAGS)


run2: image
	qemu-system-i386 -smp 2 $(QEMU_FLAGS)

run4: image
	qemu-system-i386 -smp 4 $(QEMU_FLAGS)
run5: image
	qemu-system-i386 -smp 5 $(QEMU_FLAGS)
run6: image
	qemu-system-i386 -smp 6 $(QEMU_FLAGS)
run7: image
	qemu-system-i386 -smp 7 $(QEMU_FLAGS)


run8: image
	qemu-system-i386 -smp 8 $(QEMU_FLAGS)
