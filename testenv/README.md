# `testenv`

This is a QEMU test environment. To run this, you need to have 
`qemu-system-riscv64` installed on your machine. On Ubuntu, this is provided
by:

```
sudo apt install qemu-system-misc
```

Once this is installed, you can run the qemu enviornment with 
`./qemu-testenv.sh`. The `./build-rv-only` script in the root directory of this
repo automatically places the `demo` binary inside of `rootfs.ext2`. You should
be able to simply run `./demo` from `root`'s home directory.

The access credentials for the QEMU image are `root:root`.
