#!/bin/sh

sudo umount ./fs
sudo mount ./rootfs.img ./fs
# mv ./fs/root/*.bin result