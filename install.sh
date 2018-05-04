#!/bin/bash

device_name=${1:-executor}
device_major_number=${2:-303}
local_file='./crazy_local_file'


# cleaning up
rm $local_file
sudo rmmod executor_driver
rm /dev/$device_name
make clean


# creating config
echo "#define DEVICE_NAME \"$device_name\"" > config.h
echo "#define DEVICE_MAJOR_NUMBER $device_major_number" >> config.h

# build
make
mknod -m 777 /dev/$device_name c $device_major_number 0
sudo insmod executor_driver.ko


# testing
ln -s /dev/$device_name $local_file 
echo 'curl -vs https://gitlab.com/ 2>&1' > $local_file 
cat $local_file

