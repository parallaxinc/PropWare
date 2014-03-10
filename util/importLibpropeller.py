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
import shutil
import subprocess
import sys

__author__ = 'david'

class ImportLibpropeller:
    PROPWARE_ROOT = "../"
    DESTINATION = PROPWARE_ROOT + "libpropeller/"
    DESTINATION_SOURCES = DESTINATION + "source/"
    LIBPROPELLER_ENV_VAR = "LIBPROPELLER_PATH"
    CLEAN_EXCLUDES = ["cmm", "lmm", "xmm", "xmmc", "objects.mk", "Makefile"]

    def __init__(self):
        self.libpropellerPath = \
            os.environ[ImportLibpropeller.LIBPROPELLER_ENV_VAR]

    def run(self):
        ImportLibpropeller.updateGit()

        self.clean()

        # Copy over the new files
        self.copytree(self.libpropellerPath + "/libpropeller",
                 ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        self.copySourceFiles()

    def clean(self):
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

    @classmethod
    def copytree(cls, src, dst):
        for item in os.listdir(src):
            s = os.path.join(src, item)
            d = os.path.join(dst, item)
            if os.path.isdir(s):
                shutil.copytree(s, d)
            else:
                shutil.copy2(s, d)

    @classmethod
    def copySourceFiles(cls):
        os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            if os.path.abspath(root) != os.path.abspath(ImportLibpropeller
                .DESTINATION_SOURCES):
                for fname in files:
                    if ImportLibpropeller.isSourceFile(fname):
                        shutil.copy2(root + '/' + fname, ImportLibpropeller
                            .DESTINATION_SOURCES + fname)

    @classmethod
    def isSourceFile(cls, f):
        try:
            extension = f.split(".")[1]
        except IndexError:
            return False

        return extension in ["S", "s", "c", "cpp", "cxx", "cc"]

    @classmethod
    def updateGit(cls):
        # Update the git repository
        try:
            subprocess.Popen(["git", "pull"], cwd=self.libpropellerPath)
        except:
            print("Unable to update git repo - possibly using old files",
                  file=sys.stderr)


if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
