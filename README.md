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
  

# Installation
Pretty simple:   

    $ make
    $ sudo make install
    $ sudo modprobe aaadev
    $ cat /dev/aaadev0
    AAAAAAAAAAAAAAAAAAAAAAAAAA....

# Credits   
This project was created based on [Kernel Documentation](https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html) and [Oleg Kutkov](https://olegkutkov.me/2018/03/14/simple-linux-character-device-driver/) blog.
