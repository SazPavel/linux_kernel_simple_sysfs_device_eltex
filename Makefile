CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
TARGET = simple_sysfs_device
CC = gcc
CCFLAGS = -Wall
CCNAME = main
obj-m := $(TARGET).o

.PHONY: all clean fullclean
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(CC) $(CCNAME).c -o $(CCNAME) $(CCFLAGS)
clean:
	@rm -f *.o *.mod.c *.order
fullclean: clean
	@rm *.ko *.symvers $(CCNAME)