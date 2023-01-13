.. SPDX-License-Identifier: GPL-2.0+

U-Boot for JetHub J80
======================

JetHome Jethub H1 (http://jethome.ru/jethub-h1) is a home automation
controller manufactured by JetHome with the following specifications:

 - Amlogic S905W (ARM Cortex-A53) quad-core up to 1.5GHz
 - No video out
 - 1GB DDR3
 - 8/16GB eMMC flash
 - 2 x USB 2.0
 - 1 x 10/100Mbps ethernet
 - SDIO WiFi / Bluetooth RTL8822CS IEEE 802.11a/b/g/n/ac, Bluetooth 5.0.
 - TI CC2538 + CC2592 Zigbee Wireless Module with up to 20dBm output
   power and Zigbee 3.0 support.
 - MicroSD 2.x/3.x/4.x DS/HS cards.
 - 1 x gpio LED
 - ADC user Button
 - DC source 5V microUSB
 - Square plastic case

U-Boot compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make jethub_j80_defconfig
    $ make

Image creation
--------------

For simplified usage, pleaser refer to :doc:`pre-generated-fip` with codename `jethub-j80`

Amlogic doesn't provide sources for the firmware and for tools needed
to create the bootloader image, so it is necessary to obtain binaries
from the git tree published by the board vendor:

.. code-block:: bash

    $ git clone https://github.com/jethome-ru/jethub-aml-tools jethub-u-boot
    $ cd jethub-u-boot
    $ export FIPDIR=$PWD

Go back to mainline U-Boot source tree then :

.. code-block:: bash

    $ mkdir fip

    $ cp $FIPDIR/j80/bl2.bin fip/
    $ cp $FIPDIR/j80/acs.bin fip/
    $ cp $FIPDIR/j80/bl21.bin fip/
    $ cp $FIPDIR/j80/bl30.bin fip/
    $ cp $FIPDIR/j80/bl301.bin fip/
    $ cp $FIPDIR/j80/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin

    $ $FIPDIR/blx_fix.sh \
        fip/bl30.bin \
        fip/zero_tmp \
        fip/bl30_zero.bin \
        fip/bl301.bin \
        fip/bl301_zero.bin \
        fip/bl30_new.bin \
        bl30

    $ python $FIPDIR/acs_tool.pyc fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0

    $ $FIPDIR/blx_fix.sh \
        fip/bl2_acs.bin \
        fip/zero_tmp \
        fip/bl2_zero.bin \
        fip/bl21.bin \
        fip/bl21_zero.bin \
        fip/bl2_new.bin \
        bl2

    $ $FIPDIR/j80/aml_encrypt_gxl --bl3enc --input fip/bl30_new.bin
    $ $FIPDIR/j80/aml_encrypt_gxl --bl3enc --input fip/bl31.img
    $ $FIPDIR/j80/aml_encrypt_gxl --bl3enc --input fip/bl33.bin --compress lz4
    $ $FIPDIR/j80/aml_encrypt_gxl --bl2sig --input fip/bl2_new.bin --output fip/bl2.n.bin.sig
    $ $FIPDIR/j80/aml_encrypt_gxl --bootmk \
                --output fip/u-boot.bin \
                --bl2 fip/bl2.n.bin.sig \
                --bl30 fip/bl30_new.bin.enc \
                --bl31 fip/bl31.img.enc \
                --bl33 fip/bl33.bin.enc

and then write the image to SD/eMMC with:

.. code-block:: bash

    $ DEV=/dev/your_sd_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=444
