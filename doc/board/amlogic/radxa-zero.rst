.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Radxa Zero
=====================

Radxa Zero is a small form factor SBC based on the Amlogic S905Y2
chipset that ships in a number of RAM/eMMC configurations:

Boards with 512MB/1GB LPDDR4 RAM have no eMMC storage and BCM43436
wireless (2.4GHz b/g/n) while 2GB/4GB boards have 8/16/32/64/128GB
eMMC storage and BCM4345 wireless (2.4/5GHz a/b/g/n/ac).

- Amlogic S905Y2 quad-core Cortex-A53
- Mali G31-MP2 GPU
- HDMI 2.1 output (micro)
- 1x USB 2.0 port - Type C (OTG)
- 1x USB 3.0 port - Type C (Host)
- 1x micro SD Card slot
- 40 Pin GPIO header

Schematics are available on the manufacturer website:

https://dl.radxa.com/zero/docs/hw/RADAX_ZERO_V13_SCH_20210309.pdf

U-Boot compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make radxa-zero_defconfig
    $ make

Image creation
--------------

For simplified usage, pleaser refer to :doc:`pre-generated-fip` with codename `radxa-zero`

Amlogic does not provide sources for the firmware and for tools needed
to create the bootloader image, so it is necessary to obtain them from
git trees published by the board vendor:

.. code-block:: bash

    $ git clone -b radxa-zero-v2021.07 https://github.com/radxa/u-boot.git
    $ git clone https://github.com/radxa/fip.git

    $ sudo apt-get install -y gcc-aarch64-linux-gnu device-tree-compiler libncurses5 libncurses5-dev
    $ sudo apt-get install -y bc python dosfstools flex build-essential libssl-dev mtools

    $ wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
    $ sudo tar xvf gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz -C /opt

    $ export CROSS_COMPILE=/opt/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-
    $ export ARCH=arm
    $ cd u-boot
    $ make radxa-zero_defconfig
    $ make

    $ cp u-boot.bin ../fip/radxa-zero/bl33.bin
    $ cd ../fip/radxa-zero
    $ make

This will generate:

.. code-block:: bash

    $ u-boot.bin u-boot.bin.sd.bin u-boot.bin.usb.bl2 u-boot.bin.usb.tpl

Then write the image to SD with:

.. code-block:: bash

    $ DEV=/dev/your_sd_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=444
