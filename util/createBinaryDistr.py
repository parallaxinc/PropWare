#!/usr/bin/python
# @file    createBinaryDistr.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@brief  Create a binary distribution of PropWare
"""

from __future__ import print_function
import sys
import os
from glob import glob
from zipfile import ZipFile
import subprocess
from time import sleep
from shutil import copy2
import argparse

from propwareImporter import importAll
import propwareUtils


class CreateBinaryDistr:
    PROPWARE_ROOT = ""
    ARCHIVE_FILE_NAME = "PropWare_%s.zip"
    WHITELISTED_FILES = ["CMakeLists.txt", "Doxyfile", "README", "run_all_tests", "run_unit"]
    WHITELIST_EXTENSIONS = ["c", "s", "cpp", "cxx", "cc", "h", "a", "dox", "md", "py", "pl", "elf", "rb",
                            "jpg", "lang", "pdf", "png"]
    BLACKLISTED_DIRECTORIES = ["docs", ".idea", ".settings", ".git", propwareUtils.DOWNLOADS_DIRECTORY]
    BRANCHES = ["master", "development", "release-2.0", "release-2.0-nightly"]
    TAGS = ["v1.1", "v1.2", "v2.0-beta1", "v2.0-beta2"]
    CURRENT_SUGGESTION = "release-2.0"
    MAKE_COMPILE = ["make", "-j4", "--silent"]
    CMAKE_GENERATE_MAKEFILES = ["cmake", "."]

    def __init__(self):
        self.successes = []

        propwareUtils.checkProperWorkingDirectory()

        # Get the current path and truncate "/util" from the end (therefore resulting in PropWare's root)
        CreateBinaryDistr.PROPWARE_ROOT = os.getcwd()[:-5]

        # The remainder of this script needs to be run from the PropWare root directory
        os.chdir("..")
        CreateBinaryDistr.cleanOldArchives()

    def run(self, branches, areTags=False):
        assert (isinstance(branches, list))
        assert (isinstance(areTags, bool))

        branches.sort()
        self.successes = []

        try:
            # Let's start by cleaning some stuff up if possible
            CreateBinaryDistr.clean()
            CreateBinaryDistr.cleanUntracked()

            for branch in branches:
                self.runInBranch(branch, areTags)
        finally:
            CreateBinaryDistr.attemptCleanExit()

        self.printSummary(branches)

    def runInBranch(self, branch, isTag):
        # Attempt to checkout the next branch
        if 0 == CreateBinaryDistr.checkout(branch, isTag):
            if CreateBinaryDistr.isBranchWithImporter():
                importAll()

            # Compile the static libraries and example projects
            CreateBinaryDistr.compile()

            # Generate the archive file name
            archiveName = CreateBinaryDistr.ARCHIVE_FILE_NAME % branch
            with ZipFile(archiveName, 'w') as archive:
                # Add all whitelisted files (see CreateBinaryDistr.isWhitelisted() ) so long as they are not within a
                # blacklisted directory
                for root, dirs, files in os.walk(CreateBinaryDistr.PROPWARE_ROOT):
                    # First, determine whether or not the directory we are iterating over is blacklisted...
                    rootList = root.split('/')
                    try:
                        # Currently, the only blacklisted directories are direct children of the PropWare root
                        isBlacklisted = rootList[1] in CreateBinaryDistr.BLACKLISTED_DIRECTORIES
                    except IndexError:
                        # Obviously, if rootList[1] throws an error, we aren't looking at a blacklisted directory
                        isBlacklisted = False

                    # Finally, check each file within the directory and see if it is whitelisted
                    if not isBlacklisted:
                        for file in files:
                            if self.isWhitelisted(file):
                                archive.write(root + '/' + file)

            self.successes.append(branch)
            if CreateBinaryDistr.CURRENT_SUGGESTION == branch:
                self.successes.append("current")
                copy2(archiveName, CreateBinaryDistr.ARCHIVE_FILE_NAME % "current")

        # Clean again. Cleaning is good. You should clean your house more often too!
        CreateBinaryDistr.clean()
        CreateBinaryDistr.cleanUntracked()

    @staticmethod
    def cleanOldArchives():
        files = glob("./PropWare_*.zip")
        for f in files:
            os.remove(f)

    @staticmethod
    def clean():
        if CreateBinaryDistr.isCMakeBranch():
            # Try to generate the Make files so that we can clean stuff up...
            if 0 != subprocess.call(CreateBinaryDistr.CMAKE_GENERATE_MAKEFILES, cwd=CreateBinaryDistr.PROPWARE_ROOT):
                # But if it fails, no biggy. Just move on
                return
            sys.stdout.flush()

        subprocess.call(["make", "clean", "--silent"], cwd=CreateBinaryDistr.PROPWARE_ROOT)
        sys.stdout.flush()

        # Not all branches have the simple_clean target, so it's no big deal if it fails
        try:
            subprocess.check_output(["make", "simple_clean", "--silent"], cwd=CreateBinaryDistr.PROPWARE_ROOT)
        except subprocess.CalledProcessError as e:
            if 2 != e.returncode:
                raise e

    @staticmethod
    def cleanUntracked():
        # If we've made it this far without failure, then clean all untracked files (leftovers from the previous branch)
        subprocess.call(["git", "clean", "-fd"], cwd=CreateBinaryDistr.PROPWARE_ROOT)

    @staticmethod
    def checkout(branch, isTag=False):
        assert (isinstance(isTag, bool))

        try:
            CreateBinaryDistr.cleanUntracked()
            subprocess.check_output(["git", "checkout", branch])
        except subprocess.CalledProcessError:
            print("Failed to checkout " + branch, file=sys.stderr)
            return 1

        if not isTag:
            try:
                subprocess.check_output(["git", "pull"])
            except subprocess.CalledProcessError:
                print("Failed to pull latest sources", file=sys.stderr)
                return 1

        return 0

    @staticmethod
    def compile():
        # Determine if Makefile or CMake branch
        if CreateBinaryDistr.isCMakeBranch():
            if 0 != subprocess.call(CreateBinaryDistr.CMAKE_GENERATE_MAKEFILES, cwd=CreateBinaryDistr.PROPWARE_ROOT):
                sys.stdout.flush()
                raise MakeErrorException()

        sys.stdout.flush()

        if 0 != subprocess.call(CreateBinaryDistr.MAKE_COMPILE, cwd=CreateBinaryDistr.PROPWARE_ROOT):
            sys.stdout.flush()
            raise MakeErrorException()

        sys.stdout.flush()

    @staticmethod
    def isWhitelisted(filename):
        if filename in CreateBinaryDistr.WHITELISTED_FILES:
            return True
        else:
            filename = filename.split('.')
            if 2 == len(filename) and 0 != len(filename[0]):
                if filename[1].lower() in CreateBinaryDistr.WHITELIST_EXTENSIONS:
                    return True

        return False

    @staticmethod
    def attemptCleanExit():
        try:
            CreateBinaryDistr.cleanUntracked()
            subprocess.check_output(["git", "checkout", CreateBinaryDistr.CURRENT_SUGGESTION])
        except subprocess.CalledProcessError as e:
            print("Failed to return git repository to 'current' branch", file=sys.stderr)
            print("Caused by: " + str(e), file=sys.stderr)
            print(e.output.decode(), file=sys.stderr)

    @staticmethod
    def isCMakeBranch():
        return os.path.exists("CMakeLists.txt")

    @staticmethod
    def isBranchWithImporter():
        return os.path.exists(CreateBinaryDistr.PROPWARE_ROOT + os.sep + "propwareImporter.py")

    def printSummary(self, branches):
        # Let the stdout and stderr buffers catch up
        sleep(1)

        print("\n\nSummary:")
        self.successes.sort()
        for branch in self.successes:
            print("\tPASS: " + branch)
        for branch in branches:
            if branch not in self.successes:
                print("\tFAIL: " + branch)


class MakeErrorException(Exception):
    def __init__(self):
        pass

    def __str__(self):
        return "Make failed to finish executing"


if "__main__" == __name__:
    parser = argparse.ArgumentParser(description="Create binary distributions of all branches (and optionally tags too)"
                                                 " of PropWare")
    parser.add_argument("--tags", action="store_true",
                        help="Create binary distributions for all tagged commits as well")
    args = parser.parse_args()

    runMe = CreateBinaryDistr()
    runMe.run(CreateBinaryDistr.BRANCHES, args.tags)
