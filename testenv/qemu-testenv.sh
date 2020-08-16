#!/bin/bash

DEMOBIN="/home/brad/dev/riscv-uarch-attacks/build-riscv/src/demo"

BINARIES=($DEMOBIN)

sudo mount 

for BINARY in ${BINARIES[@]}
do
    echo $BINARY
done
              
qemu-system-riscv64 \
    -M virt \
    -kernel fw_jump.elf \
    -device loader,file=Image,addr=0x80200000 \
    -append "rootwait root=/dev/vda ro" \
    -drive file=rootfs.ext2,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 \
    -netdev user,id=net0 \
    -device virtio-net-device,netdev=net0 \
    -nographic
