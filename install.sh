#!/bin/bash

sudo rmmod simple_char_driver
rm /dev/simple_char_device
make clean
make
mknod -m 777 /dev/simple_char_device  c 303 0
sudo insmod simple_char_driver.ko

