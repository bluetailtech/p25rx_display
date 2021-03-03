# p25rx_display
Touch-lcd display for use with the P25RX receiver

Build Instructions (tested on a Debian system)

1) download GNU ARM Embedded Toolchain from here:
2) https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
3) The current firmare was compiled with version: 7-2018q2-update Release. Newer versions should be ok too.
4) Create a symbolic link to the compiler in the project directory:  'ln -s gcc-arm-none-eabi-7-2018-q2-update arm-toolchain'
5) Now you can compile the project from the project directory with:  'make'
6) Flip SW2 to the ON position before supplying power to the pcb. This will put the MCU into DFU programming mode.
7) Download the dfu-util software: http://dfu-util.sourceforge.net/
8) Program the device with dfu-util:   'dfu-util --device 0483:df11 -a 0 -s 0x08000000 -D build/p25rx_display_DFU.bin'
9) Return SW2 to the OFF position and cycle power.
