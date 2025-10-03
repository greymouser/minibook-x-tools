#!/usr/bin/env bash

modprobe -r mxc4005
cp -f mda6655-split /usr/sbin/mda6655-split
cp -f 99-mxc4005-second.rules /etc/udev/rules.d/99-mxc4005-second.rules
udevadm control --reload-rules
modprobe mxc4005
udevadm settle

for n in /sys/bus/iio/devices/iio:device*/name; do printf "%s: " "$n"; cat "$n"; done
dmesg | tail -n 10
