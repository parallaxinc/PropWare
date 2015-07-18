#!/usr/bin/python
# @file    importSimple.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@brief  Import Simple Library files
"""

import os
from shutil import copy2, rmtree

import propwareUtils


class ImportSimple:
    PROPWARE_ROOT = os.path.abspath('..') + os.sep
    CHEATER_DIR = PROPWARE_ROOT + 'simple' + os.sep
    LEARN_DOWNLOAD_LINK = 'http://david.zemon.name/downloads/Learn-current.zip'
    LEARN_PATH = PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + 'Learn' + str(os.sep)

    def __init__(self):
        self.libraries = {}
        self.sourceFiles = []

    def run(self):
        propwareUtils.check_proper_working_dir()

        ImportSimple._clean()

        library_path = self._download_learn()

        self._get_libraries(library_path)

        # Process every library that was found
        for library in self.libraries.keys():
            self._process_library(library)

    def _get_libraries(self, learn_root):
        # Get a map of all the libraries
        for category in os.listdir(learn_root):
            if os.path.isdir(learn_root + category):
                for subDir in os.listdir(learn_root + category):
                    if os.path.isdir(learn_root + category + os.sep + subDir):
                        self.libraries[subDir[3:]] = learn_root + category + os.sep + subDir

        # Manually add the libsimpletools source folder
        self.libraries['__simpletools'] = learn_root + "Utility" + os.sep + "libsimpletools" + os.sep + "source"

    def _process_library(self, library):
        library_directory = self.libraries[library] + os.sep

        demo_files = ImportSimple._get_demo_file_names(library)

        # Copy all source and header files into PropWare's cheater directory
        for dir_entry in os.listdir(library_directory):
            # Don't copy the demo files
            if dir_entry not in demo_files:
                if propwareUtils.is_src_or_hdr_file(dir_entry):
                    copy2(library_directory + dir_entry, ImportSimple.CHEATER_DIR)

                    # Keep track of all the source files so we can make an object list later
                    if propwareUtils.is_src_file(dir_entry):
                        self.sourceFiles.append(dir_entry)

            # Copy over the crazy stuff like pre-compiled spin/pasm files
            if dir_entry in propwareUtils.MEMORY_MODELS:
                abs_dir_entry = library_directory + dir_entry
                for wtf in os.listdir(abs_dir_entry):
                    if wtf.endswith(".dat") and wtf not in os.listdir(ImportSimple.CHEATER_DIR):
                        copy2(abs_dir_entry + os.sep + wtf, ImportSimple.CHEATER_DIR)
                        self.sourceFiles.append(wtf)

    @staticmethod
    def _download_learn():
        propwareUtils.init_downloads_folder(ImportSimple.PROPWARE_ROOT)

        zip_file_name = propwareUtils.download_file(ImportSimple._get_download_link(),
                                                    ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)[0]
        propwareUtils.extract(zip_file_name, ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)

        return ImportSimple.LEARN_PATH + "Simple Libraries" + os.sep

    @staticmethod
    def _clean():
        rm_list = []
        for fileName in os.listdir(ImportSimple.PROPWARE_ROOT):
            if ".h" == fileName[-2:]:
                rm_list.append(ImportSimple.PROPWARE_ROOT + fileName)

        for fileName in os.listdir(ImportSimple.CHEATER_DIR):
            if propwareUtils.is_src_file(fileName):
                rm_list.append(ImportSimple.CHEATER_DIR + fileName)

        for fileName in rm_list:
            os.remove(fileName)

        destroy_me = ImportSimple.CHEATER_DIR + os.sep + "CMakeFiles"
        if os.path.exists(destroy_me):
            rmtree(destroy_me)

        if os.path.exists(ImportSimple.LEARN_PATH):
            rmtree(ImportSimple.LEARN_PATH)

    @staticmethod
    def _get_demo_file_names(library):
        names = []
        for ext in [".c", ".h"]:
            names.append("lib" + library + ext)
        return names

    @staticmethod
    def _get_download_link():
        return ImportSimple.LEARN_DOWNLOAD_LINK


if "__main__" == __name__:
    ImportSimple().run()
