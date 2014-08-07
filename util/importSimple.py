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
    PROPWARE_ROOT = os.path.abspath("..") + os.sep
    CHEATER_DIR = PROPWARE_ROOT + "simple" + os.sep
    LEARN_DOWNLOAD_LINK = "http://learn.parallax.com/sites/default/files/content/propeller-c-tutorials/" \
                          "set-up-simpleide/Learn-folder/Learn-Folder-Updated-2014.05.14.zip"
    LEARN_PATH = PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + "Learn" + os.sep

    def __init__(self):
        self.libraries = {}
        self.sourceFiles = []

    def run(self):
        propwareUtils.checkProperWorkingDirectory()

        ImportSimple.clean()

        libraryPath = self.downloadLearn()

        self.getLibraries(libraryPath)

        # Process every library that was found
        for library in self.libraries.keys():
            self.processLibrary(library)

        # Create a one-line makefile that lists all objects to be compiled into the Simple library
        self.makeObjectList()

    def getLibraries(self, learnRoot):
        # Get a map of all the libraries
        for category in os.listdir(learnRoot):
            if os.path.isdir(learnRoot + category):
                for subDir in os.listdir(learnRoot + category):
                    if os.path.isdir(learnRoot + category + os.sep + subDir):
                        self.libraries[subDir[3:]] = learnRoot + category + os.sep + subDir

        # Manually add the libsimpletools source folder
        self.libraries['__simpletools'] = learnRoot + "Utility" + os.sep + "libsimpletools" + os.sep + "source"

    def processLibrary(self, library):
        libraryDirectory = self.libraries[library] + os.sep

        demoFiles = ImportSimple.getDemoFileNames(library)

        # Copy all source and header files into PropWare's cheater directory
        for dirEntry in os.listdir(libraryDirectory):
            # Don't copy the demo files
            if dirEntry not in demoFiles:
                if propwareUtils.isSourceFile(dirEntry):
                    copy2(libraryDirectory + dirEntry, ImportSimple.CHEATER_DIR)

                    # Keep track of all the source files so we can make an object list later
                    self.sourceFiles.append(dirEntry)

                elif propwareUtils.isHeaderFile(dirEntry):
                    copy2(libraryDirectory + dirEntry, ImportSimple.PROPWARE_ROOT)

            # Copy over the crazy stuff like pre-compiled spin/pasm files
            if dirEntry in propwareUtils.MEMORY_MODELS:
                absDirEntry = libraryDirectory + dirEntry
                for wtf in os.listdir(absDirEntry):
                    if wtf.endswith(".dat") and wtf not in os.listdir(ImportSimple.CHEATER_DIR):
                        copy2(absDirEntry + os.sep + wtf, ImportSimple.CHEATER_DIR)

    def makeObjectList(self):
        # Sort the list so that the makefile doesn't change every time this is run (the following for-loop doesn't run
        # in any guaranteed order)
        self.sourceFiles.sort()
        with open(ImportSimple.CHEATER_DIR + "simpleObjects.cmake", 'w') as f:
            f.write("set(SIMPLE_OBJECTS")
            for sourceFile in self.sourceFiles:
                f.write('\n' + ' '*8 + '../' + sourceFile)
            f.write(')')

    @staticmethod
    def downloadLearn():
        propwareUtils.initDownloadsFolder(ImportSimple.PROPWARE_ROOT)

        zipFileName = propwareUtils.downloadFile(ImportSimple.getDownloadLink(),
                                                 ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)[0]
        propwareUtils.extractZip(zipFileName, ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)

        return ImportSimple.LEARN_PATH + "Simple Libraries" + os.sep

    @staticmethod
    def clean():
        rmList = []
        for fileName in os.listdir(ImportSimple.PROPWARE_ROOT):
            if ".h" == fileName[-2:]:
                rmList.append(ImportSimple.PROPWARE_ROOT + fileName)

        for fileName in os.listdir(ImportSimple.CHEATER_DIR):
            if propwareUtils.isSourceFile(fileName):
                rmList.append(ImportSimple.CHEATER_DIR + fileName)

        for fileName in rmList:
            os.remove(fileName)

        destroyMe = ImportSimple.CHEATER_DIR + os.sep + "CMakeFiles"
        if os.path.exists(destroyMe):
            rmtree(destroyMe)

        if os.path.exists(ImportSimple.LEARN_PATH):
            rmtree(ImportSimple.LEARN_PATH)

    @staticmethod
    def getDemoFileNames(library):
        names = []
        for ext in [".c", ".h"]:
            names.append("lib" + library + ext)
        return names

    @staticmethod
    def getDownloadLink():
        return ImportSimple.LEARN_DOWNLOAD_LINK


if "__main__" == __name__:
    ImportSimple().run()
