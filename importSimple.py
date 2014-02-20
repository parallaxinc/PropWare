#/usr/bin/python
# @file    importSimple.py
# @author  David Zemon
# @project PropWare
#
# Created with: PyCharm Community Edition

"""
@brief  Import Simple Library files
"""

import os
from shutil import copy2


class ImportSimple:
    PROPWARE_ROOT = "./"
    CHEATER_DIR = "simple/"
    LEARN_ENV_VAR = "PROP_LEARN_PATH"

    def __init__(self):
        self.libraries = {}

    def run(self):
        self.getLibraries(ImportSimple.getRootPath())

        # Process every library that was found
        for library in self.libraries.keys():
            ImportSimple.processLibrary(library, self.libraries[library] + '/')

    def getLibraries(self, rootPath):
        # Get a map of all the libraries
        for categoryDir in os.listdir(rootPath):
            if os.path.isdir(rootPath + categoryDir):
                for subDir in os.listdir(rootPath + categoryDir):
                    if os.path.isdir(rootPath + categoryDir + '/' + subDir):
                        self.libraries[subDir[3:]] = rootPath + categoryDir + '/' + subDir

    @staticmethod
    def getRootPath():
        # Get the root path without the stupid extra slashes
        propLearnPath = os.environ[ImportSimple.LEARN_ENV_VAR].split('\\')
        return ''.join(propLearnPath) + '/Simple Libraries/'

    @staticmethod
    def isSourceOrHeaderFile(f):
        return ImportSimple.isSourceFile(f) or ImportSimple.isHeaderFile(f)

    @staticmethod
    def isSourceFile(f):
        assert (isinstance(f, str))
        return f[-2:] == ".c"

    @staticmethod
    def isHeaderFile(f):
        assert (isinstance(f, str))
        return f[-2:] == ".h"

    @staticmethod
    def getDemoFileNames(library):
        names = []
        for ext in [".c", ".h"]:
            names.append("lib" + library + ext)
        return names

    @staticmethod
    def processLibrary(library, libraryDirectory):
        # Copy all source and header files into PropWare's cheater directory
        for f in os.listdir(libraryDirectory):
            # Don't copy the demo files
            if f not in ImportSimple.getDemoFileNames(library):
                if ImportSimple.isSourceFile(f):
                    copy2(libraryDirectory + f, ImportSimple.PROPWARE_ROOT + ImportSimple.CHEATER_DIR)
                elif ImportSimple.isHeaderFile(f):
                    copy2(libraryDirectory + f, ImportSimple.PROPWARE_ROOT)


if "__main__" == __name__:
    ImportSimple().run()
