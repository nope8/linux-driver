#!/bin/bash
mknod /dev/mydev c 250 0
chmod 777 /dev/mydev
insmod timer2.ko


