
BINARY        := aaadev
CFLAGS        := -Wall -Wextra

obj-m += $(BINARY).o

define local-vars
SEC_BOOT_PRIV_KEY     = /private/SECBOOT_SIGN_MOD
SEC_BOOT_PUB_KEY      = /private/SECBOOT_SIGN_MOD.der
KERNEL_MODULES_PATH   = /lib/modules/$(shell uname -r)
KERNEL_SOURCES_PATH   = /usr/src/linux-headers-$(shell uname -r)
endef

define arm64-linux-5.4-vars
ARCH                  = arm64
CROSS_COMPILE         = aarch64-linux-gnu-
CC                    = aarch64-linux-android30-clang
CXX                   = aarch64-linux-android30-clang++
KERNEL_MODULES_PATH   = /home/dukpt/linux-modules-5.4.277/lib/modules/5.4.277DUKPT-00038-g929096094cf8
KERNEL_SOURCES_PATH   = /home/dukpt/linux-src-5.4.277
endef

#
# TODO
# linux-headers-5.15.x
#

all:
	@echo "Please choose: make local || make arm64-linux-5.4"
	@echo "make local            - build module for local kernel;  make local-install;  make local-uninstall"
	@echo "make arm64-linux-5.4  - build cross compile kernel targeting Aarch64"


local-vars-target:
	$(eval $(local-vars))
	@echo "SEC_BOOT_PRIV_KEY=$(SEC_BOOT_PRIV_KEY)"
	@echo "SEC_BOOT_PUB_KEY=$(SEC_BOOT_PUB_KEY)"
	@echo "KERNEL_MODULES_PATH=$(KERNEL_MODULES_PATH)"
	@echo "KERNEL_SOURCES_PATH=$(KERNEL_SOURCES_PATH)"


local: local-vars-target
	make -C $(KERNEL_MODULES_PATH)/build M=$(PWD) modules
	# if you don't have Secure Boot just delete the line below
	$(KERNEL_SOURCES_PATH)/scripts/sign-file sha256 $(SEC_BOOT_PRIV_KEY) $(SEC_BOOT_PUB_KEY) $(PWD)/$(BINARY).ko


local-clean: local-vars-target
	make -C $(KERNEL_MODULES_PATH)/build M=$(PWD) clean


clean: local-clean


local-install: local-vars-target
	#cp $(BINARY).ko $(KERNEL_MODULES_PATH)/kernel/drivers/char
	# when I tried install -s (strip) I've got ERROR: could not insert 'aaadev': Exec format error
	install -m 755 -o root -g root $(BINARY).ko $(KERNEL_MODULES_PATH)/kernel/drivers/char
	$(KERNEL_SOURCES_PATH)/scripts/sign-file sha256 $(SEC_BOOT_PRIV_KEY) $(SEC_BOOT_PUB_KEY) $(KERNEL_MODULES_PATH)/kernel/drivers/char/$(BINARY).ko
	depmod -a


local-uninstall: local-vars-target
	rm $(KERNEL_MODULES_PATH)/kernel/drivers/char/$(BINARY).ko
	depmod -a


arm64-linux-5.4-vars-target:
	$(eval $(arm64-linux-5.4-vars))
	@echo "KERNEL_MODULES_PATH=$(KERNEL_MODULES_PATH)"
	@echo "KERNEL_SOURCES_PATH=$(KERNEL_SOURCES_PATH)"


arm64-linux-5.4: arm64-linux-5.4-vars-target
	make -C $(KERNEL_MODULES_PATH)/build M=$(PWD) ARCH=$(ARCH) CC=$(CC) CXX=$(CXX) CROSS_COMPILE=$(CROSS_COMPILE) modules


arm64-linux-5.4-clean: arm64-linux-5.4-vars-target
	make -C $(KERNEL_MODULES_PATH)/build M=$(PWD) clean



.PHONY: all  clean  local-clean  local-uninstall  local-vars-target  arm64-linux-5.4-vars-target  arm64-linux-5.4-clean

