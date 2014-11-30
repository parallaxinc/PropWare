#!/usr/bin/python
# @file    datSymbolConverter
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
Make this work correctly:
    propeller-elf-objcopy
        -Ibinary
        -Bpropeller
        -Opropeller-elf-gcc
        --redefine-sym _binary_cmm_pst_dat_start=_binary_pst_dat_start
        --redefine-sym _binary_cmm_pst_dat_end=_binary_pst_dat_end
        --redefine-sym _binary_cmm_pst_dat_size=_binary_pst_dat_size
        cmm/pst.dat
        cmm/pst_firmware.o
"""
import argparse
import os
import shutil
import subprocess
import tempfile


def parse_args():
    parser = argparse.ArgumentParser(
        description='Rename the symbols in an object file to fit the way SimpleIDE names them')
    parser.add_argument('--objcopy', dest='objcopy', required=True,
                        help='Define the path to propeller-elf-objcopy')
    parser.add_argument('-i', '--input', dest='input', required=True,
                        help='Create binary distributions for all tagged commits as well')
    parser.add_argument('-o', '--output', dest='output', required=True, help='Output file path')
    parser.add_argument('-v', '--verbose', dest='verbose', required=False, action='store', nargs='?',
                        type=bool, help='Enable verbose output')
    return parser.parse_args()


if '__main__' == __name__:
    args = parse_args()

    input_file_path = args.input
    output_file_path = args.output
    objcopy = args.objcopy

    tmp = tempfile.gettempdir()
    input_file_name = os.path.split(input_file_path)[1]
    output_file_name = os.path.split(output_file_path)[1]
    tmp_input_file = tmp + str(os.sep) + input_file_name
    tmp_output_file = tmp + str(os.sep) + output_file_name

    if os.path.exists(tmp_input_file):
        os.remove(tmp_input_file)

    if args.verbose:
        print('cp %s %s' % (input_file_path, tmp_input_file))
    shutil.copy2(input_file_path, tmp_input_file)

    cmd = [objcopy, '-Ibinary', '-Opropeller-elf-gcc', '-Bpropeller', input_file_name, output_file_name]
    if args.verbose:
        print(' '.join(cmd))
    ret = subprocess.call(cmd, cwd=tmp)
    if 0 != ret:
        raise Exception('Renaming symbols failed for ' + input_file_path)

    if args.verbose:
        print('cp %s %s' % (tmp_output_file, output_file_path))
    shutil.copy2(tmp_output_file, output_file_path)
