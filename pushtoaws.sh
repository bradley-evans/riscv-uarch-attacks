#!/bin/bash

set -e

KEYFILE="/home/brad/.ssh/firesim.pem"

DEMOBIN="/home/brad/dev/riscv-uarch-attacks/build-riscv/bin/demo"
L1PPBIN="/home/brad/dev/riscv-uarch-attacks/build-riscv/bin/l1pp_datacollect"
HVMBIN="/home/brad/dev/riscv-uarch-attacks/build-riscv/bin/hitvsmiss_datacollect"


BINARIES=($DEMOBIN $L1PPBIN $HVMBIN)

./build-rv-only

if [ $# -eq 0 ]
then
    echo "You need to supply an IP address for the remote AWS server."
    exit 1
fi

echo "Pushing RISC-V demo build to AWS remote at $1."

for BINARY in ${BINARIES[@]}
do
    echo "    - Pushing $BINARY."
    scp -i $KEYFILE $BINARY centos@$1:~
    ssh -i $KEYFILE centos@$1 sudo mount firesim/deploy/workloads/linux-uniform/br-base.img /rvimg/
    ssh -i $KEYFILE centos@$1 sudo cp $(basename $BINARY) /rvimg/root/
    ssh -i $KEYFILE centos@$1 sudo umount /rvimg
    echo "    - Good."
done

echo "Push complete."
