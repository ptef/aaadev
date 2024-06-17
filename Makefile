
# edit the 3 variables below
SEC_BOOT_PRIV_KEY  := /private/SECBOOT_SIGN_MOD
SEC_BOOT_PUB_KEY   := /private/SECBOOT_SIGN_MOD.der
KERNEL_PATH        := /lib/modules/$(shell uname -r)

# uncomment below just in case you want to use another compiler rather then
# the default installed in your system
#CC = /opt/llvm/bin/clang
#CXX = /opt/llvm/bin/clang++
#AR = /opt/llvm/bin/llvm-ar
#RANLIB = /opt/llvm/bin/llvm-ranlib
#AS = /opt/llvm/bin/llvm-as
#LLVM_CONFIG = /opt/llvm/bin/llvm-config

BINARY        := aaadev
CFLAGS        := -Wall -Wextra

obj-m += $(BINARY).o

all:
	make -C $(KERNEL_PATH)/build M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH)/build M=$(PWD) clean

install:
	cp $(BINARY).ko $(KERNEL_PATH)/kernel/drivers/char
	/usr/src/linux-headers-$(shell uname -r)/scripts/sign-file sha256 $(SEC_BOOT_PRIV_KEY) $(SEC_BOOT_PUB_KEY) $(BINARY).ko
	depmod -a

uninstall:
	rm $(KERNEL_PATH)/kernel/drivers/char/$(BINARY).ko
	depmod -a

