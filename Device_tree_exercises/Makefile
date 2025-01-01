obj-m += manhtd24_driver.o 

# Set kernel source path (path to your Orange Pi's kernel source)
KDIR := /home/hinokami/Documents/orangepi-build/kernel/orange-pi-6.1-sun50iw9

# Set the current directory path
PWD := $(shell pwd)

# Cross-compiler prefix (based on your toolchain)
CROSS_COMPILE := /usr/bin/aarch64-linux-gnu-

# Architecture (ARM64 for Orange Pi Zero 2W)
ARCH := arm64

# Build the module
all:
	make -C $(KDIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) modules

# Clean up the build
clean:
	make -C $(KDIR) M=$(PWD) clean