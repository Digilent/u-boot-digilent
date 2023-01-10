# Copyright (c) 2018, Xilinx Inc.
#
# Siva Durga Prasad Paladugu
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import re
import random
import u_boot_utils

"""
Note: This test relies on boardenv_* containing configuration values to define
the network available and files to be used for testing. Without this, this test
will be automatically skipped.

For example:

# True if a DHCP server is attached to the network, and should be tested.
env__net_dhcp_server = True

# A list of environment variables that should be set in order to configure a
# static IP. In this test case we atleast need serverip for performing tftpb
# to get required files.
env__net_static_env_vars = [
    ("ipaddr", "10.0.0.100"),
    ("netmask", "255.255.255.0"),
    ("serverip", "10.0.0.1"),
]

# Details regarding the files that may be read from a TFTP server. .
env__zynq_rsa_readable_file = {
    "fn": "zynq_rsa_image.bin",
    "fninvalid": "zynq_rsa_image_invalid.bin",
    "srcaddr": 0x1000000,
}
"""

import test_net

def zynq_rsa_pre_commands(u_boot_console):

    output = u_boot_console.run_command('print modeboot')
    if not "modeboot=" in output:
       pytest.skip('bootmode cannnot be determined')
    m = re.search('modeboot=(.+?)boot', output)
    if not m:
       pytest.skip('bootmode cannnot be determined')
    bootmode=m.group(1)
    if bootmode == "jtag":
       pytest.skip('skipping due to jtag bootmode')

@pytest.mark.buildconfigspec('cmd_zynq_rsa')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net')
# Can be tested on board xhd-zc702-2 - efuse must be blown
@pytest.mark.xfail
def test_zynq_rsa_image(u_boot_console):

    zynq_rsa_pre_commands(u_boot_console)
    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)

    f = u_boot_console.config.env.get('env__zynq_rsa_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = u_boot_console.run_command('tftpboot %x %s' % (srcaddr, fn))
    assert expected_tftp in output

    expected_zynqrsa = 'zynq rsa <baseaddr>'
    output = u_boot_console.run_command('zynq rsa %x ' % (srcaddr))
    assert expected_zynqrsa not in output

@pytest.mark.buildconfigspec('cmd_zynq_rsa')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net')
# Can be tested on board xhd-zc702-2 - efuse must be blown
@pytest.mark.xfail
def test_zynq_rsa_image_invalid(u_boot_console):

    zynq_rsa_pre_commands(u_boot_console)
    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)

    f = u_boot_console.config.env.get('env__zynq_rsa_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    expected_tftp = 'Bytes transferred = '
    fninvalid = f['fninvalid']
    output = u_boot_console.run_command('tftpboot %x %s' % (srcaddr, fninvalid))
    assert expected_tftp in output

    expected_zynqrsa = 'zynq rsa <baseaddr>'
    output = u_boot_console.run_command('zynq rsa %x ' % (srcaddr))
    assert expected_zynqrsa in output
