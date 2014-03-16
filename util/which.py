#/usr/bin/python
# File:    ${file}
# Author:  David Zemon
# Project: PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
__author__ = 'david'

import os


def which(program):
    def is_exe(filePath):
        return os.path.isfile(filePath) and os.access(filePath, os.X_OK)

    directory, fileName = os.path.split(program)
    if directory:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    # If we haven't returned anything yet, that means the program doesn't exist
    return None
