# About This Project

This project is being developed by Bradley Evans, a graduate student
at the Department of Computer Science and Engineering, Bourns College
of Engineering, University of California-Riverside. This codebase was
created to satisfy requirements for the Master's Degree in Computer
Engineering at that university. Bradley Evans is advised by Prof.
Nael Abu-Ghazaleh.

This codebase is a portion of a thesis titled "The Attack Surface of
RISC-V Processors," currently unpublished (as of Sept 2020).

# Introduction

TODO

# Root Directory Structure

| item              | description                    |
|-------------------|--------------------------------|
| doc/              | Doxygen-generated documentation and thesis LaTeX. |
| prototypes/       | Small code prototypes used to build some functions. |
| src/              | The actual source code. |
| testenv/          | A QEMU-based test environment. |
| tests/            | Unit tests. |
| results/          | Some results obtained, analysis scripts. |
| build             | Build script. |
| build-rv-only     | Builds the project for RISC-V only. |
| pushtoaws.sh      | Script to push a new build to AWS FireSim. |

# Usage

## Recommendations and Requirements

This repository was tested on a machine running Ubuntu 20.04 with a build of
the `riscv-gnu-toolchain` available from their 
[GitHub repository](https://github.com/riscv/riscv-gnu-toolchain). You need to
have this repository set and the `$RISCV` environment variable set to point to
the compiled toolchain. This will use `riscv64-unknown-linux-gnu-gcc` as its
compiler for RISC-V binaries.

## Building this Project

Demonstration binaries, named `demo`, are built for the native architecture
and for the RISC-V architecture by invoking `./build` from the root directory
of this repository.

The `demo` binary will be found in `build-default/src/demo` and 
`build-riscv/src/demo` respectively.

This project uses `cmake` and the `riscv-gnu-toolchain` to perform its builds. Both are required to make use of this repository, with a valid `$RISCV` environment variable set.

## Running RISC-V Binaries

### In QEMU

QEMU does not simulate caches. However, we found it useful to test that a
binary will run at all before going to the trouble of moving a binary over to
RISC-V hardware or into some other cycle accurate simualtion.

The `build-rv-only` script will open an `ext2` image in `testenv` for you and
place a demo binary inside of there at `/root/demo`. There is a README inside
of `testenv` that explains what to do from there.

### On fpga-zynq

We had a lot of initial success testing the RISC-V binaries we generated on a
[Zedboard](http://zedboard.org/product/zedboard) using the 
[`fpga-zynq`](https://github.com/ucb-bar/fpga-zynq) repository to instantiate a
Rocket core on the FPGA. That is one option.

### On FireSim

Berkeley Architecture Research has developed the 
[FireSim platform](https://docs.fires.im/en/latest/#) to allow RISC-V
testing to be performed on AWS-F1 instances. The `push-to-aws` script is meant
to push the demo binary to a FireSim manager instance, as set up per the basic
setup guide in the FireSim documentation. You can run it with
```
./push-to-aws $FIRESIM-MANAGER-IP
```
