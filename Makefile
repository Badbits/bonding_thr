#
# Makefile for the Ethernet Bonding driver
#


obj-$(CONFIG_BONDING) += bonding.o

bonding-objs := bond_main.o bond_3ad.o bond_alb.o bond_sysfs.o bond_debugfs.o bond_thr.o


EXTRA_CFLAGS += -DCONFIG_PROC_FS
KVERSION=$(shell uname -r)

all:
		make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules




