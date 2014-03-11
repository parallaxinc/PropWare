#/usr/bin/python
# @file    importLibpropeller.py
# @author  David Zemon
# @project PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
import os
import shutil
import subprocess
import sys

__author__ = 'david'


class ImportLibpropeller:
    PROPWARE_ROOT = "../"
    DESTINATION = PROPWARE_ROOT + "libpropeller/"
    SOURCE_DROPBOX = "source"
    DESTINATION_SOURCES = DESTINATION + SOURCE_DROPBOX + '/'
    LIBPROPELLER_ENV_VAR = "LIBPROPELLER_PATH"
    SOURCE_OBJECT_LIST = "libpropellerObjects.mk"
    CLEAN_EXCLUDES = ["cmm", "lmm", "xmm", "xmmc", "Makefile", SOURCE_DROPBOX]

    def __init__(self):
        self.libpropellerPath = os.environ[ImportLibpropeller.LIBPROPELLER_ENV_VAR]
        self.sourceFiles = []

    def run(self):
        self.updateGit()

        self.clean()

        # Copy over the new files
        self.copytree(self.libpropellerPath + "/libpropeller", ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        self.copySourceFiles()

        self.makeObjectList()

    @staticmethod
    def clean():
        """
        Clean the old directory
        """
        if os.path.exists(ImportLibpropeller.DESTINATION):
            for entry in os.listdir(ImportLibpropeller.DESTINATION):
                if entry not in ImportLibpropeller.CLEAN_EXCLUDES:
                    removable = ImportLibpropeller.DESTINATION + entry
                    if os.path.isdir(removable):
                        shutil.rmtree(removable)
                    else:
                        os.remove(removable)

    @staticmethod
    def copytree(src, dst):
        for item in os.listdir(src):
            s = os.path.join(src, item)
            d = os.path.join(dst, item)
            if os.path.isdir(s):
                shutil.copytree(s, d)
            else:
                shutil.copy2(s, d)

    def copySourceFiles(self):
        # noinspection PyBroadException
        try:
            os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)
        except:  # When Python3 is standard, we can specify FileExistsError... but it's not in Python2
            pass  # Don't care if the file already exists

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            if os.path.abspath(root) != os.path.abspath(ImportLibpropeller.DESTINATION_SOURCES):
                for f in files:
                    if ImportLibpropeller.isAsmFile(f):
                        shutil.copy2(root + '/' + f, ImportLibpropeller.DESTINATION_SOURCES + f)
                        self.sourceFiles.append(f)

    def makeObjectList(self):
        # Sort the list so that the makefile doesn't change every time this is run (the following for-loop doesn't run
        # in any guaranteed order)
        self.sourceFiles.sort()
        with open(ImportLibpropeller.DESTINATION_SOURCES + ImportLibpropeller.SOURCE_OBJECT_LIST, 'w') as f:
            f.write("OBJS = ")
            for sourceFile in self.sourceFiles:
                sourceFile = sourceFile.split('.')[0]  # Remove the extension
                f.write(sourceFile + ".o ")

    @staticmethod
    def isAsmFile(f):
        try:
            extension = f.split(".")[1]
        except IndexError:
            return False

        return extension in ["S", "s"]

    def updateGit(self):
        # Update the git repository
        # noinspection PyBroadException
        try:
            subprocess.Popen(["git", "pull"], cwd=self.libpropellerPath)
        except:
            print("Unable to update git repo - possibly using old files", file=sys.stderr)


if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
