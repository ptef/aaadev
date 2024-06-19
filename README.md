# aaadev
Linux kernel character device that outputs "AAAA"

This is a very simple kernel module that creates 2 character devices `/dev/aaadev0`and `/dev/aaadev1` that prints "AAAA" when reading. It's similar to `/dev/urandom` with no randomness.   
In the code it's defined as an array of 4 bytes `char data[4] = { 'A', 'A', 'A', 'A' }`.   

    $ cat /dev/aaadev0
    AAAAAAAAAAAAAAAAAAAA...

When writing, the value of `data[4]`can be overwritten to user desired 4-byte size value:  

    $ echo -ne "DIE\x0a" > /dev/aaadev0
    $ head /dev/aaadev0 
    DIE
    DIE
    DIE
    DIE
    DIE
    DIE
    DIE
    DIE
    DIE
    DIE
  

# Setup
After cloning the repo, open Makefile and edit the variables below:

    SEC_BOOT_PRIV_KEY     - enter the path to your private key already enrolled into your Secure Boot key vault
    SEC_BOOT_PUB_KEY      - public key
    KERNEL_SOURCES_PATH   - path to the kernel source you want to build the driver
    KERNEL_MODULES_PATH   - the same for the modules


The following commands can help you to discover your keys:
    mokutil --list-enrolled
    mokutil --list-new

If you haven't your key already enrolled, you can use the following commands to (1) generate the pair and (2) import the public:
    (1) openssl req -new -x509 -newkey rsa:4096 -keyout MOK -outform DER -out MOK.der -nodes -days 36500 -subj "/CN=My Kernel Module Signing/"
    (2) sudo mokutil --import MOK.der

After importing you'll must reboot your machine and enter the unlock password during the boot. Don't forget to choose the option to enroll the key you've just imported!

# Installation
After changing the Makefile the installation is simple:   

    $ make
    $ make local
    $ sudo make local-install
    $ /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 /private/MOK /private/MOK.der /research/ptef/aaadev/aaadev.ko
    $ sudo modprobe aaadev
    $ cat /dev/aaadev0
    AAAAAAAAAAAAAAAAAAAAAAAAAA....
    $ echo "OIE" > /dev/aaadev0
    $ head -1 /dev/aaadev0
    OIE

For the Aarch64 I'm targeting Android Linux (AOSP) so I also have Android Studio and NDK installed and exported. This is required for the clang compiler I'm using for the arm64-linux-5.4 target inside Makefile:

    export ANDROID_NDK_HOME="/home/dukpt/Android/Sdk/ndk/27.0.11902837";
    export PATH="${PATH}:${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64/bin";

If you plan to cross build for another target you'll have to adapt the Makefile.

# Uninstallation

    $ sudo rmmod aaadev
    $ sudo make local-uninstall

# Credits
This project was created based on [Kernel Documentation](https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html) and [Oleg Kutkov](https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/) blog.
