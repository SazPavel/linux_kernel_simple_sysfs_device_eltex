CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
TARGET = simple_sysfs_device
obj-m := $(TARGET).o

.PHONY: all clean fullclean
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@rm -f *.o *.mod.c *.order
fullclean: clean
	@rm *.ko *.symvers