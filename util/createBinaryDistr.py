#/usr/bin/python
# @file    createBinaryDistr.py
# @author  David Zemon
# @project PropWare
#
# Created with: PyCharm Community Edition

"""
@brief  Create a binary distribution of PropWare
"""

import os
import time
from glob import glob
from zipfile import ZipFile

class CreateBinaryDistr:
    PROPWARE_ROOT = "../"
    SIMPLE_LIB_PATH = "simple/"
    EXAMPLES_PATH = "Examples/"
    ARCHIVE_FILE_NAME = ""
    MEM_MODS = ["cmm", "lmm", "xmm", "xmmc"]
    PROPWARE_LIB_STR = "libPropWare_%s.a"
    SIMPLE_LIB_STR = "libSimple_%s.a"

    def __init__(self):
        self.archive = None
        CreateBinaryDistr.ARCHIVE_FILE_NAME = "PropWare_%s.zip" % time.strftime("%Y-%m-%d")

    def run(self):
        os.chdir(CreateBinaryDistr.PROPWARE_ROOT)

        CreateBinaryDistr.clean()
        CreateBinaryDistr.compile()

        with ZipFile(CreateBinaryDistr.ARCHIVE_FILE_NAME, 'w') as self.archive:
            self.addSourceFiles()
            self.addPropWareLibs()
            self.addSimpleLibs()
            self.addExamples()

    def addSourceFiles(self):
        # Add all files in the root directory
        rootDirList = glob("./*")
        for entry in rootDirList:
            if os.path.isfile(entry) and entry[-3:] != "zip":
                self.archive.write(entry)

        # Add all files in the simple library directory
        simpleFileList = glob(CreateBinaryDistr.SIMPLE_LIB_PATH + "*")
        for entry in simpleFileList:
            if os.path.isfile(entry):
                self.archive.write(entry)

    def addPropWareLibs(self):
        for memMode in CreateBinaryDistr.MEM_MODS:
            libName = CreateBinaryDistr.PROPWARE_LIB_STR % memMode
            self.archive.write(memMode + '/' + libName)

    def addSimpleLibs(self):
        for memMode in CreateBinaryDistr.MEM_MODS:
            simpleLibPath = CreateBinaryDistr.SIMPLE_LIB_PATH + memMode + '/'
            libName = CreateBinaryDistr.SIMPLE_LIB_STR % memMode
            self.archive.write(simpleLibPath + libName)

    def addExamples(self):
        # Add everything in the Examples folder
        for root, subdirs, files in os.walk(CreateBinaryDistr.EXAMPLES_PATH):
            for f in files:
                self.archive.write(os.path.join(root, f))

    @staticmethod
    def clean():
        files = glob("./PropWare_*.zip")
        for f in files:
            os.remove(f)

        os.system("make clean")
        os.system("make simple_clean")

    @staticmethod
    def compile():
        if 0 != os.system("make -j4"):
            raise MakeFailure()
class MakeFailure(Exception):
    def __init__(self):
        pass

    def __str__(self):
        return "Make failed to finish executing"

if "__main__" == __name__:
    runMe = CreateBinaryDistr()
    runMe.run()
