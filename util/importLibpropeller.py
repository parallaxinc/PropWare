#!/usr/bin/python3
# @file    importLibpropeller.py
# @author  David Zemon
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
    SOURCE_OBJECT_LIST = "libpropellerObjects.cmake"
    CLEAN_EXCLUDES = ["cog", "cmm", "lmm", "xmm-split", "xmm-single", "xmmc", "CMakeLists.txt", "libpropeller.cmake"]
    WHITELISTED_SOURCE_FILES = ["numbers.cpp"]
    LIBPROPELLER_PATH = PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + "libpropeller" + os.sep

    def __init__(self):
        self.sourceFiles = []

    def run(self):
        propwareUtils.check_proper_working_dir()

        self._clean()

        propwareUtils.init_downloads_folder(self.PROPWARE_ROOT)

        self._create_and_update_git()

        # Copy over the new files
        propwareUtils.copytree(self.LIBPROPELLER_PATH + os.sep + "libpropeller", self.DESTINATION)

    def _create_and_update_git(self):
        # Ensure git exists in the path
        if not propwareUtils.which("git"):
            print("Looks like I can't update the git repository for libpropeller. Sorry!", file=sys.stderr)
            print("Caused by: 'git' is not in the PATH", file=sys.stderr)
        else:
            try:
                # If the git repository doesn't exist, create it
                if not os.path.exists(self.LIBPROPELLER_PATH):
                    subprocess.check_output("git clone https://github.com/libpropeller/libpropeller.git",
                                            cwd=self.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY,
                                            shell=True)
                # Otherwise, update the git repository
                else:
                    subprocess.check_output("git pull", cwd=self.LIBPROPELLER_PATH, shell=True)
            except subprocess.CalledProcessError as e:
                print("Looks like I can't clone or update the git repository for libpropeller. Sorry!", file=sys.stderr)
                print("Caused by: " + str(e), file=sys.stderr)
                print(e.output.decode(), file=sys.stderr)

    @classmethod
    def _clean(cls):
        """
        Clean the old directory
        """
        if os.path.exists(cls.DESTINATION):
            for entry in os.listdir(cls.DESTINATION):
                if cls.SOURCE_DROPBOX == entry:
                    for root, dirs, files in os.walk(cls.DESTINATION + entry):
                        for fileName in files:
                            if fileName not in cls.CLEAN_EXCLUDES:
                                os.remove(root + os.sep + fileName)
                    destroy_me = cls.DESTINATION + entry + os.sep + "CMakeFiles"
                    if os.path.exists(destroy_me):
                        shutil.rmtree(destroy_me)
                elif entry not in cls.CLEAN_EXCLUDES:
                    removable = cls.DESTINATION + entry
                    if os.path.isdir(removable):
                        shutil.rmtree(removable)
                    else:
                        os.remove(removable)


if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
