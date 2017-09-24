# Early Boot Utils User Guide

This repository contains the following tools for help with "early boot - late attach". 

1. `dump_rsc_table` - prints the resource table of an ELF binary
2. `check_late_props` - checks whether late attach properties are set on the device tree correctly.
3. `print_macros_for_mlo` - prints the macros that need to be defined in MLO to correspond to the device tree being used.

## Building the tools

The tools in this repository are built with [CMake](https://cmake.org/).

1. Install dependencies.

    ~~~{.bash}
    $ sudo apt-get install cmake libfdt-dev
    ~~~

2. Clone the git repository.

    ~~~{.bash}
    $ git clone git://git.ti.com/glsdk/earlyboot-utils.git
    ~~~

3. Build `earlyboot-utils` and install them to the desired location.

    ~~~{.bash}
    $ cd earlyboot-utils
    $ mkdir -p build
    $ cd build
    $ cmake -DCMAKE_INSTALL_PREFIX=~/.local/ ..
    $ make install
    ~~~

    This will install the tools to `~/.local/bin`. You can change the
    location according to your preference.

When you run the tools below, please make sure to include their
location in the `PATH`

~~~{.bash}
$ export PATH=$PATH:~/.local/bin
$ dump_rsc_table dra7-ipu2-fw.xem4.ipumm-fw
~~~

or invoke them by their absolute path.

~~~{.bash}
$ ~/.local/bin/dump_rsc_table dra7-ipu2-fw.xem4.ipumm-fw
~~~


The tools can also be cross compiled for the target by defining a
toolchain file. Please see

<http://processors.wiki.ti.com/index.php/Processor_SDK_Linux_Automotive_Application_Notes#CMake>

## Running the tools

### Dumping Resource Table

~~~{.bash}
$ dump_rsc_table dra7-ipu2-fw.xem4.ipumm-fw
Loaded file dra7-ipu2-fw.xem4.ipumm-fw

dra7-ipu2-fw.xem4.ipumm-fw found; size is 3743108 bytes
Resource table attributes
                 ver :    1
                 num :   17
         reserved[0] :    0
         reserved[1] :    0
           offset[0] :   84

 [ 0] :       vdev rsc: id 7, dfeatures 1, cfg len 0, 2 vrings
             vring rsc: num 256 pa 0x0 da 0x60000000
             vring rsc: num 256 pa 0x0 da 0x60004000
 [ 1] :   carveout rsc: pa 0x00000000, da 0x00000000, len 0x00600000, flags 0x00000000
 [ 2] :   carveout rsc: pa 0x00000000, da 0x80000000, len 0x02b00000, flags 0x00000000
 [ 3] :   carveout rsc: pa 0x00000000, da 0x9f000000, len 0x00100000, flags 0x00000000
 [ 4] :      trace rsc: da 0x9f000000, len 0x8000
 [ 5] :     devmem rsc: pa 0x95800000, da 0x60000000, len 0x00100000
 [ 6] :     devmem rsc: pa 0x60000000, da 0xa0000000, len 0x10000000
 [ 7] :     devmem rsc: pa 0x70000000, da 0xb0000000, len 0x08000000
 [ 8] :     devmem rsc: pa 0x78000000, da 0xb8000000, len 0x08000000
 [ 9] :     devmem rsc: pa 0x4a000000, da 0x6a000000, len 0x01000000
 [10] :     devmem rsc: pa 0x48000000, da 0x68000000, len 0x00200000
 [11] :     devmem rsc: pa 0x48400000, da 0x68400000, len 0x00400000
 [12] :     devmem rsc: pa 0x48800000, da 0x68800000, len 0x00800000
 [13] :     devmem rsc: pa 0x54000000, da 0x74000000, len 0x01000000
 [14] :     devmem rsc: pa 0x5a000000, da 0x7a000000, len 0x01000000
 [15] :     devmem rsc: pa 0x5b000000, da 0x7b000000, len 0x01000000
 [16] :     devmem rsc: pa 0x4e000000, da 0x6e000000, len 0x00100000
~~~

### Check DTB for late attach attributes

~~~{.bash}
$ check_late_props /tftp/dra7-evm.dtb
Loaded file /tftp/dra7-evm.dtb

/tftp/dra7-evm.dtb found; size is 120049 bytes
fdt length is 120049

 ipu1:       ipu1:       ti,late-attach not found
 ipu1:       ipu1:   ti,no-idle-on-init not found
 ipu1:       ipu1:  ti,no-reset-on-init not found
 ipu1:    timer11:       ti,late-attach not found
 ipu1:    timer11:   ti,no-idle-on-init not found
 ipu1:    timer11:  ti,no-reset-on-init not found
 ipu1:   mmu_ipu1:       ti,late-attach not found
 ipu1:   mmu_ipu1:   ti,no-idle-on-init not found
 ipu1:   mmu_ipu1:  ti,no-reset-on-init not found
 ipu1:     timer7:       ti,late-attach not found
 ipu1:     timer7:   ti,no-idle-on-init not found
 ipu1:     timer7:  ti,no-reset-on-init not found
 ipu1:     timer8:       ti,late-attach not found
 ipu1:     timer8:   ti,no-idle-on-init not found
 ipu1:     timer8:  ti,no-reset-on-init not found

...

 dsp2:  mmu1_dsp2:  ti,no-reset-on-init not found
 dsp2:    timer13:       ti,late-attach not found
 dsp2:    timer13:   ti,no-idle-on-init not found
 dsp2:    timer13:  ti,no-reset-on-init not found
~~~

###  Print location macros needed by MLO

~~~{.bash}
$ print_macros_for_mlo dra7-evm-lcd-lg-late-attach.dtb
Loaded file dra7-evm-lcd-lg-late-attach.dtb

dra7-evm-lcd-lg-late-attach.dtb found; size is 110045 bytes
fdt length is 110045
#define DRA7_RPROC_CMA_BASE_IPU1                      0x9d000000
#define DRA7_RPROC_CMA_BASE_IPU2                      0x95800000
#define DRA7_RPROC_CMA_BASE_DSP1                      0x99000000
#define DRA7_RPROC_CMA_BASE_DSP2                      0x9f000000

#define DRA7_RPROC_CMA_SIZE_IPU1                      0x02000000
#define DRA7_RPROC_CMA_SIZE_IPU2                      0x03800000
#define DRA7_RPROC_CMA_SIZE_DSP1                      0x04000000
#define DRA7_RPROC_CMA_SIZE_DSP2                      0x00800000

#define DRA7_PGTBL_BASE_ADDR                          0xbfc00000
~~~
