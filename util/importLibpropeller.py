#!/usr/bin/python
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

    def __init__(self):
        self.sourceFiles = []
        self.LIBPROPELLER_PATH = ImportLibpropeller.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + \
                                 "libpropeller" + os.sep

    def run(self):
        propwareUtils.check_proper_working_dir()

        self._clean()

        propwareUtils.init_downloads_folder(ImportLibpropeller.PROPWARE_ROOT)

        self._create_and_update_git()

        # Copy over the new files
        propwareUtils.copytree(self.LIBPROPELLER_PATH + os.sep + "libpropeller", ImportLibpropeller.DESTINATION)

        # Copy all source files into a source directory so we can create the
        # library
        self._copy_src_files()

        self._make_obj_list()

    def _copy_src_files(self):
        # If the source directory doesn't exist yet, create it
        if not os.path.exists(ImportLibpropeller.DESTINATION_SOURCES):
            os.mkdir(ImportLibpropeller.DESTINATION_SOURCES)

        for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION):
            # Skip the root git directory and move into its subdirectories
            # noinspection PyBroadException
            try:
                # noinspection PyUnresolvedReferences
                libpropeller_dst_root = os.path.samefile(root, ImportLibpropeller.DESTINATION_SOURCES)
            except Exception:  # This should be NameError... but that wasn't properly catching the exception... :'(
                libpropeller_dst_root = os.path.normcase(root) == os.path.normcase(
                    ImportLibpropeller.DESTINATION_SOURCES)

            if not libpropeller_dst_root:
                for f in files:
                    if self._is_worthy_file(f):
                        shutil.copy2(root + str(os.sep) + f, ImportLibpropeller.DESTINATION_SOURCES + f)
                        self.sourceFiles.append(f)

    def _make_obj_list(self):
        # Sort the list so that the makefile doesn't change every time this is run (the following for-loop doesn't run
        # in any guaranteed order)
        self.sourceFiles.sort()
        with open(ImportLibpropeller.DESTINATION_SOURCES + ImportLibpropeller.SOURCE_OBJECT_LIST, 'w') as f:
            f.write("set(LIBPROPELLER_OBJECTS")
            for sourceFile in self.sourceFiles:
                f.write('\n' + ' ' * 8 + '..' + str(os.sep) + sourceFile)
            f.write(')')

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
                                            cwd=ImportLibpropeller.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY,
                                            shell=True)
                # Otherwise, update the git repository
                else:
                    subprocess.check_output("git pull", cwd=self.LIBPROPELLER_PATH, shell=True)
            except subprocess.CalledProcessError as e:
                print("Looks like I can't clone or update the git repository for libpropeller. Sorry!", file=sys.stderr)
                print("Caused by: " + str(e), file=sys.stderr)
                print(e.output.decode(), file=sys.stderr)

    def _is_worthy_file(self, file_name):
        is_whitelisted = file_name in ImportLibpropeller.WHITELISTED_SOURCE_FILES
        is_asm = propwareUtils.is_asm_file(file_name)
        is_new = file_name not in self.sourceFiles
        is_not_symlink = not propwareUtils.is_symbolic_link_on_windows(file_name)
        return (is_whitelisted or is_asm) and is_new and is_not_symlink

    @staticmethod
    def _clean():
        """
        Clean the old directory
        """
        if os.path.exists(ImportLibpropeller.DESTINATION):
            for entry in os.listdir(ImportLibpropeller.DESTINATION):
                if ImportLibpropeller.SOURCE_DROPBOX == entry:
                    for root, dirs, files in os.walk(ImportLibpropeller.DESTINATION + entry):
                        for fileName in files:
                            if fileName not in ImportLibpropeller.CLEAN_EXCLUDES:
                                os.remove(root + os.sep + fileName)
                    destroy_me = ImportLibpropeller.DESTINATION + entry + os.sep + "CMakeFiles"
                    if os.path.exists(destroy_me):
                        shutil.rmtree(destroy_me)
                elif entry not in ImportLibpropeller.CLEAN_EXCLUDES:
                    removable = ImportLibpropeller.DESTINATION + entry
                    if os.path.isdir(removable):
                        shutil.rmtree(removable)
                    else:
                        os.remove(removable)


if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
