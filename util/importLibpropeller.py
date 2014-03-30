#!/usr/bin/python
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

import propwareUtils

__author__ = 'david'


class ImportLibpropeller:
    PROPWARE_ROOT = os.path.abspath("..") + os.sep
    DESTINATION = PROPWARE_ROOT + "libpropeller" + os.sep
    SOURCE_DROPBOX = "source"
    DESTINATION_SOURCES = DESTINATION + SOURCE_DROPBOX + os.sep
    SOURCE_OBJECT_LIST = "libpropellerObjects.mk"
    CLEAN_EXCLUDES = ["cmm", "lmm", "xmm", "xmmc", "Makefile", "libpropeller.mk"]
    WHITELISTED_SOURCE_FILES = ["numbers.cpp"]

    def __init__(self):
        self.sourceFiles = []
        self.LIBPROPELLER_PATH = ImportLibpropeller.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + \
                                 "libpropeller" + os.sep

    def run(self):
        propwareUtils.checkProperWorkingDirectory()

        self.clean()

        propwareUtils.initDownloadsFolder(ImportLibpropeller.PROPWARE_ROOT)

        self.createAndUpdateGit()

        # Copy over the new files
        propwareUtils.copytree(self.LIBPROPELLER_PATH + os.sep + "libpropeller", ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        self.copySourceFiles()

        self.makeObjectList()

    def copySourceFiles(self):
        # If the source directory doesn't exist yet, create it
        if not os.path.exists(ImportLibpropeller.DESTINATION_SOURCES):
            os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            # Skip the root git directory and move into its subdirectories
            if not os.path.samefile(root, ImportLibpropeller.DESTINATION_SOURCES):
                for f in files:
                    if self.isWorthyFile(f):
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

    def createAndUpdateGit(self):
        # Ensure git exists in the path
        if not propwareUtils.which("git"):
            print("Looks like I can't update the git repository for libpropeller. Sorry!", file=sys.stderr)
            print("Caused by: 'git' is not in the PATH", file=sys.stderr)
        else:
            try:
                # If the git repository doesn't exist, create it
                if not os.path.exists(self.LIBPROPELLER_PATH):
                    subprocess.check_output("git clone https://github.com/libpropeller/libpropeller.git",
                                            cwd=ImportLibpropeller.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY,
                                            shell=True)
                # Otherwise, update the git repository
                else:
                    subprocess.check_output("git pull", cwd=self.LIBPROPELLER_PATH, shell=True)
            except subprocess.CalledProcessError as e:
                print("Looks like I can't clone or update the git repository for libpropeller. Sorry!", file=sys.stderr)
                print("Caused by: " + str(e), file=sys.stderr)
                print(e.output.decode(), file=sys.stderr)

    def isWorthyFile(self, fileName):
        isWhiteListed = fileName in ImportLibpropeller.WHITELISTED_SOURCE_FILES
        isAssembly = propwareUtils.isAsmFile(fileName)
        isNew = fileName not in self.sourceFiles
        return (isWhiteListed or isAssembly) and isNew

    @staticmethod
    def clean():
        """
        Clean the old directory
        """
        if os.path.exists(ImportLibpropeller.DESTINATION):
            for entry in os.listdir(ImportLibpropeller.DESTINATION):
                if ImportLibpropeller.SOURCE_DROPBOX == entry:
                    for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION + entry):
                        for fileName in files:
                            if fileName not in ImportLibpropeller.CLEAN_EXCLUDES:
                                os.remove(root + '/' + fileName)
                elif entry not in ImportLibpropeller.CLEAN_EXCLUDES:
                    removable = ImportLibpropeller.DESTINATION + entry
                    if os.path.isdir(removable):
                        shutil.rmtree(removable)
                    else:
                        os.remove(removable)


if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
