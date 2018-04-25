#!/bin/bash

sudo rmmod simple_char_driver
make
sudo insmod simple_char_driver.ko

