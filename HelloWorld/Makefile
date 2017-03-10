obj-m += hello-1.o
obj-m += hello-2.o

CURR=$(shell uname -r)
TARG=/home/student/felabs/sysdev/tinysystem/linux-2.6.34
target:
	make -C $(TARG) M=$(PWD) modules

host:
	make -C /lib/modules/$(CURR)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(CURR)/build M=$(PWD) clean
