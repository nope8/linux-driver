ifeq ($(KERNELRELEASE),)

#KERNELDIR ?= /media/MQ/farsight/S5PC100/kernel/linux-2.6.35/
#CC = /media/MQ/toolchain/arm-2010.09/bin/arm-none-linux-gnueabi-gcc
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	cp *.ko /source/rootfs/

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

#test:test.c


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* module* test

.PHONY: modules modules_install clean

else
    obj-m := s5pc100_platform.o
endif

