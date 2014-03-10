#/usr/bin/python
# @file    importLibpropeller.py
# @author  David Zemon
# @project PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
import os
from shutil import copytree, rmtree, copy2
import subprocess
import sys
from filecmp import dircmp

__author__ = 'david'


class ImportLibpropeller:
    PROPWARE_ROOT = "../"
    DESTINATION = PROPWARE_ROOT + "libpropeller/"
    DESTINATION_SOURCES = DESTINATION + "source/"
    LIBPROPELLER_ENV_VAR = "LIBPROPELLER_PATH"

    @staticmethod
    def run():
        libpropellerPath = os.environ[ImportLibpropeller.LIBPROPELLER_ENV_VAR]

        # Update the git repository
        try:
            subprocess.Popen(["git", "pull"], cwd=libpropellerPath)
        except:
            print("Unable to update git repo - possibly using old files",
                  file=sys.stderr)

        # Clean the old directory
        rmtree(ImportLibpropeller.DESTINATION)

        # Copy over the new one
        copytree(libpropellerPath + "/libpropeller",
                 ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        ImportLibpropeller.copySourceFiles()

    @staticmethod
    def copySourceFiles():
        os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            if not dircmp(root, ImportLibpropeller.DESTINATION_SOURCES):
                for fname in files:
                    if ImportLibpropeller.isSourceFile(fname):
                        copy2(root + '/' + fname, ImportLibpropeller
                            .DESTINATION_SOURCES + fname)

    @staticmethod
    def isSourceFile(f):
        try:
            extension = f.split(".")[1]
        except IndexError:
            return False

        return extension in ["S", "s", "c", "cpp", "cxx", "cc"]

if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
