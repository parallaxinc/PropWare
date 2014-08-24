#!/usr/bin/python
# @file    cmakeScrubber
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
import os
import shutil

import propwareUtils

if "__main__" == __name__:
    propwareUtils.check_proper_working_dir()
    os.chdir("..")

    deleteMes = []
    for subdir, dirs, files in os.walk("."):
        if subdir.endswith("CMakeFiles"):
            deleteMes.append(subdir)

    for folder in deleteMes:
        try:
            shutil.rmtree(folder)
            print(folder)
        except OSError:
            pass

    deleteMes = []
    for subdir, dirs, files in os.walk("."):
        for file in files:
            if file in ["CMakeCache.txt", "cmake_install.cmake"]:
                deleteMes.append(subdir + os.sep + file)

    for file in deleteMes:
        print(file)
        os.remove(file)
