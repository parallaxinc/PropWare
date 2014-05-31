#/usr/bin/python
# @file    importSimple.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@brief  Import Simple Library files
"""

import os
from shutil import copy2
import zipfile
import shutil

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

    def getLibraries(self, rootPath):
        # Get a map of all the libraries
        for categoryDir in os.listdir(rootPath):
            if os.path.isdir(rootPath + categoryDir):
                for subDir in os.listdir(rootPath + categoryDir):
                    if os.path.isdir(rootPath + categoryDir + '/' + subDir):
                        self.libraries[subDir[3:]] = rootPath + categoryDir + '/' + subDir

        # Manually add the libsimpletools source folder
        self.libraries['__simpletools'] = rootPath + "Utility/libsimpletools/source"

    def processLibrary(self, library):
        libraryDirectory = self.libraries[library] + '/'

        demoFiles = ImportSimple.getDemoFileNames(library)

        # Copy all source and header files into PropWare's cheater directory
        for f in os.listdir(libraryDirectory):
            # Don't copy the demo files
            if f not in demoFiles:
                if propwareUtils.isSourceFile(f):
                    copy2(libraryDirectory + f, ImportSimple.CHEATER_DIR)

                    # Keep track of all the source files so we can make an object list later
                    self.sourceFiles.append(f)

                elif propwareUtils.isHeaderFile(f):
                    copy2(libraryDirectory + f, ImportSimple.PROPWARE_ROOT)

    def makeObjectList(self):
        # Sort the list so that the makefile doesn't change every time this is run (the following for-loop doesn't run
        # in any guaranteed order)
        self.sourceFiles.sort()
        with open(ImportSimple.CHEATER_DIR + "simpleObjects.cmake", 'w') as f:
            f.write("set(SIMPLE_OBJECTS")
            for sourceFile in self.sourceFiles:
                f.write('\n' + ' '*8 + '../' + sourceFile[:-2])
            f.write(')')

    @staticmethod
    def downloadLearn():
        propwareUtils.initDownloadsFolder(ImportSimple.PROPWARE_ROOT)

        zipFileName = propwareUtils.downloadFile(ImportSimple.getDownloadLink(),
                                                 ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)[0]
        zipFile = zipfile.ZipFile(zipFileName, mode='r')
        zipFile.extractall(ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY)

        return ImportSimple.PROPWARE_ROOT + propwareUtils.DOWNLOADS_DIRECTORY + "Learn" + os.sep + "Simple Libraries"\
               + os.sep

    @staticmethod
    def clean():
        rmList = []
        for fileName in os.listdir(ImportSimple.PROPWARE_ROOT):
            if ".h" == fileName[-2:]:
                rmList.append(ImportSimple.PROPWARE_ROOT + fileName)

        for fileName in os.listdir(ImportSimple.CHEATER_DIR):
            if fileName[-2:] in [".c", "cpp"]:
                rmList.append(ImportSimple.CHEATER_DIR + fileName)

        for fileName in rmList:
            os.remove(fileName)

        destroyMe = ImportSimple.CHEATER_DIR + os.sep + "CMakeFiles"
        if os.path.exists(destroyMe):
            shutil.rmtree(destroyMe)

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
