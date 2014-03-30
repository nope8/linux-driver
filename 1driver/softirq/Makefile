$(warning KERNELRELEASE=$(KERNELRELEASE))
ifeq ($(KERNELRELEASE),)

#KERNELDIR ?= /home/tom/fs2416/linux-2.6.39/
KERNELDIR ?= /lib/modules/$(shell uname -r)/build  

PWD := $(shell pwd)

modules:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 
modules_install:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
		rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules* read write test_ioctl test_lseek

.PHONY: modules modules_install clean

else
	    obj-m := softirq.o
endif

