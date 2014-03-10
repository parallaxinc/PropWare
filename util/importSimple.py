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
    PROPWARE_ROOT = "../"
    CHEATER_DIR = PROPWARE_ROOT + "simple/"
    LEARN_ENV_VAR = "PROP_LEARN_PATH"

    def __init__(self):
        self.libraries = {}
        self.sourceFiles = []

    def run(self):
        ImportSimple.clean()

        self.getLibraries(ImportSimple.getRootPath())

        # Process every library that was found
        for library in self.libraries.keys():
            self.processLibrary(library)

        # Create a one-line makefile that lists all objects to be compiled into the Simple library
        self.makeObjectList()

    def getLibraries(self, rootPath):
        # Get a map of all the libraries
        for categoryDir in os.listdir(rootPath):
            if os.path.isdir(rootPath + categoryDir):
                for subDir in os.listdir(rootPath + categoryDir):
                    if os.path.isdir(rootPath + categoryDir + '/' + subDir):
                        self.libraries[subDir[3:]] = rootPath + categoryDir + '/' + subDir

        # Manually add the libsimpletools source folder
        self.libraries['__simpletools'] = rootPath + "Utility/libsimpletools/source"

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

    def processLibrary(self, library):
        libraryDirectory = self.libraries[library] + '/'

        demoFiles = ImportSimple.getDemoFileNames(library)

        # Copy all source and header files into PropWare's cheater directory
        for f in os.listdir(libraryDirectory):
            # Don't copy the demo files
            if f not in demoFiles:
                if ImportSimple.isSourceFile(f):
                    copy2(libraryDirectory + f, ImportSimple.CHEATER_DIR)

                    # Keep track of all the source files so we can make an object list later
                    self.sourceFiles.append(f)

                elif ImportSimple.isHeaderFile(f):
                    copy2(libraryDirectory + f, ImportSimple.PROPWARE_ROOT)

    def makeObjectList(self):
        # Sort the list so that the makefile doesn't change every time this is run (the following for-loop doesn't run
        # in any guaranteed order)
        self.sourceFiles.sort()
        with open(ImportSimple.CHEATER_DIR + "simpleObjects.mk", 'w') as f:
            f.write("OBJS = ")
            for sourceFile in self.sourceFiles:
                f.write(sourceFile[:-1] + "o ")

    @staticmethod
    def clean():
        rmList = []
        for fname in os.listdir(ImportSimple.PROPWARE_ROOT):
            if ".h" == fname[-2:]:
                rmList.append(ImportSimple.PROPWARE_ROOT + fname)

        for fname in os.listdir(ImportSimple.CHEATER_DIR):
            if ".c" == fname[-2:]:
                rmList.append(ImportSimple.CHEATER_DIR + fname)

        for fname in rmList:
            os.remove(fname)

if "__main__" == __name__:
    ImportSimple().run()
