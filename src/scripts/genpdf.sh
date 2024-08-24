#!/bin/sh

mv ./result/*.bin ./result/ash
# sudo umount ./fs
# sudo mount ./rootfs.img ./fs
cp ./fs/root/*.bin result
./oscomp_repo/src/yat_trace/st-draw ./result/*.bin