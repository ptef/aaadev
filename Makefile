
#CC = /opt/llvm/bin/clang
#CXX = /opt/llvm/bin/clang++
#AR = /opt/llvm/bin/llvm-ar
#RANLIB = /opt/llvm/bin/llvm-ranlib
#AS = /opt/llvm/bin/llvm-as
#LLVM_CONFIG = /opt/llvm/bin/llvm-config

BINARY      := aaadev
CFLAGS      := -Wall -Wextra


obj-m += $(BINARY).o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	cp $(BINARY).ko /lib/modules/$(shell uname -r)/kernel/drivers/char
	depmod -a

uninstall:
	rm /lib/modules/$(shell uname -r)/kernel/drivers/char/$(BINARY).ko
	depmod -a

