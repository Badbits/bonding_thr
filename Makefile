#
# Makefile for the Ethernet Bonding driver
#

obj-m = bonding.o

obj-$(CONFIG_BONDING) += bonding.o

bonding-objs := bond_main.o bond_3ad.o bond_alb.o bond_sysfs.o bond_thr.o

ipv6-$(subst m,y,$(CONFIG_IPV6)) += bond_ipv6.o
bonding-objs += $(ipv6-y)

EXTRA_CFLAGS += -DCONFIG_PROC_FS
KVERSION=$(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
