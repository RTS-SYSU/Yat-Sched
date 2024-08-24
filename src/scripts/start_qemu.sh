#!/bin/sh

qemu-system-x86_64 \
    -kernel ./yat_sched/arch/x86_64/boot/bzImage \
    -drive file=rootfs.img,format=raw \
    -append "root=/dev/sda rw" \
    -smp 4 \
    -m 2G \
    -vnc :1