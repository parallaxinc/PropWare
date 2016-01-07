#!/usr/bin/python3
# @file    importSimple.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@brief  Import Simple Library files
"""

import os
import subprocess
import sys
from shutil import copy2, rmtree

import propwareUtils


class ImportSimple:
    PROPWARE_ROOT = os.path.abspath('..') + os.sep
    CHEATER_DIR = PROPWARE_ROOT + 'simple' + os.sep
    LEARN_PATH = PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + 'Learn' + str(os.sep)
    SIMPLE_PATH = PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + 'propsideworkspace' + os.sep
    BLACKLISTED_LIBRARIES = []
    BLACKLISTED_FILES = ['jm_time.c', 'date_time_epoch.spin', 'colorpal.c', 'colorpal.h']

    def __init__(self):
        self.libraries = {}
        self.sourceFiles = []

    def run(self):
        propwareUtils.check_proper_working_dir()

        self._clean()

        propwareUtils.init_downloads_folder(self.PROPWARE_ROOT)

        self._create_and_update_git()

        self._get_libraries(self.SIMPLE_PATH + "Learn/Simple Libraries" + os.sep)

        # Process every library that was found
        for library in self.libraries.keys():
            self._process_library(library)

    def _create_and_update_git(self):
        # Ensure git exists in the path
        if not propwareUtils.which("git"):
            print("Looks like I can't update the git repository for Simple. Sorry!", file=sys.stderr)
            print("Caused by: 'git' is not in the PATH", file=sys.stderr)
        else:
            try:
                # If the git repository doesn't exist, create it
                if not os.path.exists(self.SIMPLE_PATH):
                    subprocess.check_output("git clone https://github.com/parallaxinc/propsideworkspace.git",
                                            cwd=self.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY,
                                            shell=True)
                # Otherwise, update the git repository
                else:
                    subprocess.check_output("git pull", cwd=self.SIMPLE_PATH, shell=True)
            except subprocess.CalledProcessError as e:
                print("Looks like I can't clone or update the git repository for Simple. Sorry!", file=sys.stderr)
                print("Caused by: " + str(e), file=sys.stderr)
                print(e.output.decode(), file=sys.stderr)

    def _get_libraries(self, learn_root):
        # Get a map of all the libraries
        for category in os.listdir(learn_root):
            category_path = learn_root + category
            if os.path.isdir(category_path):
                for sub_dir in os.listdir(category_path):
                    sub_dir_path = category_path + os.sep + sub_dir
                    if os.path.isdir(sub_dir_path) and sub_dir not in self.BLACKLISTED_LIBRARIES:
                        self.libraries[sub_dir[3:]] = sub_dir_path

        # Manually add the libsimpletools source folder
        self.libraries['__simpletools'] = learn_root + "Utility" + os.sep + "libsimpletools" + os.sep + "source"

    def _process_library(self, library):
        library_directory = self.libraries[library] + os.sep

        demo_files = self._get_demo_file_names(library)

        # Copy all source and header files into PropWare's cheater directory
        for dir_entry in os.listdir(library_directory):
            # Don't copy the demo files
            if dir_entry not in demo_files:
                if propwareUtils.is_src_or_hdr_file(dir_entry) and dir_entry not in self.BLACKLISTED_FILES:
                    copy2(library_directory + dir_entry, self.CHEATER_DIR)

                    # Keep track of all the source files so we can make an object list later
                    if propwareUtils.is_src_file(dir_entry):
                        self.sourceFiles.append(dir_entry)

            # Copy over the crazy stuff like pre-compiled spin/pasm files
            if dir_entry in propwareUtils.MEMORY_MODELS:
                abs_dir_entry = library_directory + dir_entry
                for wtf in os.listdir(abs_dir_entry):
                    if wtf.endswith(".dat") and wtf not in os.listdir(self.CHEATER_DIR):
                        copy2(abs_dir_entry + os.sep + wtf, self.CHEATER_DIR)
                        self.sourceFiles.append(wtf)

    @classmethod
    def _clean(cls):
        rm_list = []
        for fileName in os.listdir(cls.PROPWARE_ROOT):
            if ".h" == fileName[-2:]:
                rm_list.append(cls.PROPWARE_ROOT + fileName)

        for fileName in os.listdir(cls.CHEATER_DIR):
            if propwareUtils.is_src_file(fileName):
                rm_list.append(cls.CHEATER_DIR + fileName)

        for fileName in rm_list:
            os.remove(fileName)

        destroy_me = cls.CHEATER_DIR + os.sep + "CMakeFiles"
        if os.path.exists(destroy_me):
            rmtree(destroy_me)

        if os.path.exists(cls.LEARN_PATH):
            rmtree(cls.LEARN_PATH)

    @classmethod
    def _get_demo_file_names(cls, library):
        names = []
        for ext in [".c", ".h"]:
            names.append("lib" + library + ext)
        return names

    @classmethod
    def _get_download_link(cls):
        return cls.LEARN_DOWNLOAD_LINK


if "__main__" == __name__:
    ImportSimple().run()
