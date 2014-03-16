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
from which import which

__author__ = 'david'


class ImportLibpropeller:
    PROPWARE_ROOT = "../"
    DESTINATION = PROPWARE_ROOT + "libpropeller/"
    SOURCE_DROPBOX = "source"
    DESTINATION_SOURCES = DESTINATION + SOURCE_DROPBOX + '/'
    SOURCE_OBJECT_LIST = "libpropellerObjects.mk"
    CLEAN_EXCLUDES = ["cmm", "lmm", "xmm", "xmmc", "Makefile", "libpropeller.mk"]

    def __init__(self, downloadsFolder):
        assert (isinstance(downloadsFolder, str))

        self.sourceFiles = []
        if downloadsFolder[:-1] not in ['/', '\\']:
            downloadsFolder += '/'
        self.DOWNLOADS_FOLDER = downloadsFolder
        self.LIBPROPELLER_PATH = downloadsFolder + "libpropeller/"

    def run(self):
        self.clean()

        self.initDownloadsFolder()

        self.createAndUpdateGit()

        # Copy over the new files
        self.copytree(self.LIBPROPELLER_PATH + "/libpropeller", ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        self.copySourceFiles()

        self.makeObjectList()

    def initDownloadsFolder(self):
        # Create the folder if it doesn't exist
        if not os.path.exists(self.DOWNLOADS_FOLDER):
            os.mkdir(self.DOWNLOADS_FOLDER)

        # If on Windows, set the hidden attribute
        if "nt" == os.name:
            winDirName = self.DOWNLOADS_FOLDER.replace('/', "\\")
            import ctypes
            FILE_ATTRIBUTE_HIDDEN = 0x02

            ret = ctypes.windll.kernel32.SetFileAttributesW(winDirName, FILE_ATTRIBUTE_HIDDEN)
            if 0 == ret:  # return code of zero indicates failure, raise Windows error
                raise ctypes.WinError()

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
        # If the source directory doesn't exist yet, create it
        if not os.path.exists(ImportLibpropeller.DESTINATION_SOURCES):
            os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            # Skip the root git directory and move into its subdirectories
            if os.path.abspath(root) != os.path.abspath(ImportLibpropeller.DESTINATION_SOURCES):
                for f in files:
                    if ImportLibpropeller.isAsmFile(f) and f not in self.sourceFiles:
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

    def createAndUpdateGit(self):
        # Ensure git exists in the path
        if not which("git"):
            print("Looks like I can't update the git repository for libpropeller. Sorry!", file=sys.stderr)
            print("Caused by: 'git' is not in the PATH", file=sys.stderr)
        else:
            try:
                # If the git repository doesn't exist, create it
                if not os.path.exists(self.LIBPROPELLER_PATH):
                    subprocess.check_output("git clone https://github.com/libpropeller/libpropeller.git",
                                            cwd=self.DOWNLOADS_FOLDER, shell=True)
                # Otherwise, update the git repository
                else:
                    subprocess.check_output("git pull", cwd=self.LIBPROPELLER_PATH, shell=True)
            except subprocess.CalledProcessError as e:
                print("Looks like I can't clone or update the git repository for libpropeller. Sorry!",
                      file=sys.stderr)
                print("Caused by: " + str(e), file=sys.stderr)
                print(e.output.decode(), file=sys.stderr)


if "__main__" == __name__:
    importer = ImportLibpropeller(ImportLibpropeller.PROPWARE_ROOT + ".external_downloads")
    importer.run()
