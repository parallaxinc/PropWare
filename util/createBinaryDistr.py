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
from importLibpropeller import ImportLibpropeller
from importSimple import ImportSimple


class CreateBinaryDistr:
    PROPWARE_SOURCES = "PropWare/"
    SIMPLE_LIB_PATH = "simple/"
    EXAMPLES_PATH = "Examples/"
    ARCHIVE_FILE_NAME = "PropWare_%s.zip"
    MEM_MODS = ["cmm", "lmm", "xmm", "xmmc"]
    PROPWARE_LIB_STR = "libPropWare_%s.a"
    SIMPLE_LIB_STR = "libSimple_%s.a"
    WHITELISTED_FILES = ["Makefile", "Doxyfile", "README", "run_all_tests", "run_unit"]
    WHITELIST_EXTENSIONS = ["c", "s", "cpp", "cxx", "cc", "h", "a", "dox", "md", "py", "pl", "elf", "txt", "rb", "jpg",
                            "lang", "pdf", "png"]
    BLACKLISTED_DIRECTORIES = ["docs", ".idea", ".settings", ".git"]

    def __init__(self):
        self.archive = None
        CreateBinaryDistr.ARCHIVE_FILE_NAME = CreateBinaryDistr.ARCHIVE_FILE_NAME % time.strftime("%Y-%m-%d")

    def run(self):
        self.checkProperInitDirectory()

        # Import libpropeller
        libpropellerImporter = ImportLibpropeller()
        libpropellerImporter.run()

        # Import simple libraries
        simpleImporter = ImportSimple()
        simpleImporter.run()

        os.chdir("..")  # File must be run from within <propware root>/util directory

        CreateBinaryDistr.clean()
        CreateBinaryDistr.compile()

        with ZipFile(CreateBinaryDistr.ARCHIVE_FILE_NAME, 'w') as self.archive:
            for root, dirs, files in os.walk('.'):
                rootList = root.split('/')
                try:
                    isBlacklisted = rootList[1] in CreateBinaryDistr.BLACKLISTED_DIRECTORIES
                except IndexError:
                    isBlacklisted = False

                if not isBlacklisted:
                    for file in files:
                        if self.isWhitelisted(file):
                            self.archive.write(root + '/' + file)

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
            raise MakeErrorException()

    @staticmethod
    def checkProperInitDirectory():
        if "createBinaryDistr.py" not in os.listdir('.'):
            raise IncorrectStartingDirectoryException()

    @staticmethod
    def isWhitelisted(filename):
        if filename in CreateBinaryDistr.WHITELISTED_FILES:
            return True
        else:
            filename = filename.split('.')
            if 2 == len(filename) and 0 != len(filename[0]):
                if filename[1].lower() in CreateBinaryDistr.WHITELIST_EXTENSIONS:
                    return True

        return False


class MakeErrorException(Exception):
    def __init__(self):
        pass

    def __str__(self):
        return "Make failed to finish executing"


class IncorrectStartingDirectoryException(Exception):
    def __init__(self):
        pass

    def __str__(self):
        return "Must be executed from within <propware root>/util"


if "__main__" == __name__:
    runMe = CreateBinaryDistr()
    runMe.run()
