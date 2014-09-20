#!/usr/bin/python
# @file    createBinaryDistr.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@brief  Create a binary distribution of PropWare
"""

from __future__ import print_function
import multiprocessing
import sys
import os
from glob import glob
from zipfile import ZipFile
import subprocess
from time import sleep
from shutil import copy2
import argparse

import propwareUtils


class CreateBinaryDistr(object):
    PROPWARE_ROOT = ""
    ARCHIVE_FILE_NAME = "PropWare_%s.zip"
    WHITELISTED_FILES = ["CMakeLists.txt", "Doxyfile", "README", "run_all_tests", "run_unit"]
    WHITELIST_EXTENSIONS = ["c", "s", "cpp", "cxx", "cc", "h", "a", "dox", "md", "py", "pl", "elf", "rb", "jpg", "lang",
                            "pdf", "png", "cmake"]
    BLACKLISTED_DIRECTORIES = ["docs", ".idea", ".settings", ".git", propwareUtils.DOWNLOADS_DIRECTORY]
    BRANCHES = ["master", "development", "release-2.0", "release-2.0-nightly"]
    TAGS = ["v1.1", "v1.2", "v2.0-beta1", "v2.0-beta2", "v2.0-beta3", "v2.0-beta4"]
    CURRENT_SUGGESTION = "release-2.0"
    MAKE_COMPILE = ["make", "-j%d" % (multiprocessing.cpu_count() if 4 >= multiprocessing.cpu_count() else 4),
                    "--silent"]
    MAKE_CLEAN_FAILED_CODE = 2
    CMAKE_GENERATE_FAILED_CODE = 1

    def __init__(self, propware_root=None):
        self.successes = []
        self.currentBranch = ""

        # Get the current path and truncate "/util" from the end (therefore resulting in PropWare's root)
        if None == propware_root:
            propwareUtils.check_proper_working_dir()
            CreateBinaryDistr.PROPWARE_ROOT = os.getcwd()[:-5]

            # The remainder of this script needs to be run from the PropWare root directory
            os.chdir("..")
        else:
            CreateBinaryDistr.PROPWARE_ROOT = propware_root

        CreateBinaryDistr._clean_old_archives()

    # noinspection PyShadowingNames
    def run(self, branches):
        assert (isinstance(branches, list))

        branches.sort()
        self.successes = []

        try:
            # Let's start by cleaning some stuff up if possible
            CreateBinaryDistr._clean()
            CreateBinaryDistr._clean_untracked()

            for branch in branches:
                self.runInBranch(branch)
        finally:
            CreateBinaryDistr._attempt_clean_exit()

        self._print_summary(branches)

    def runInBranch(self, branch):
        # Here for debugging purposes only
        self.currentBranch = branch

        # Attempt to checkout the next branch
        if 0 == CreateBinaryDistr._checkout(branch):
            if CreateBinaryDistr._is_branch_with_importer():
                os.chdir("util")
                from propwareImporter import importAll
                importAll()
                os.chdir("..")

            # Compile the static libraries and example projects
            CreateBinaryDistr._compile()

            # Generate the archive
            archive_name = CreateBinaryDistr.ARCHIVE_FILE_NAME % branch
            with ZipFile(archive_name, 'w') as archive:
                # Add all whitelisted files (see CreateBinaryDistr._is_whitelisted() ) so long as they are not within a
                # blacklisted directory
                for root, dirs, files in os.walk(CreateBinaryDistr.PROPWARE_ROOT):
                    # First, determine whether or not the directory we are iterating over is blacklisted...
                    root_list = root.split('/')
                    try:
                        # Currently, the only blacklisted directories are direct children of the PropWare root
                        is_blacklisted = root_list[1] in CreateBinaryDistr.BLACKLISTED_DIRECTORIES
                    except IndexError:
                        # Obviously, if root_list[1] throws an error, we aren't looking at a blacklisted directory
                        is_blacklisted = False

                    # Finally, check each file within the directory and see if it is whitelisted
                    if not is_blacklisted:
                        for f in files:
                            if self._is_whitelisted(f):
                                archive.write(root + '/' + f)

            self.successes.append(branch)
            if CreateBinaryDistr.CURRENT_SUGGESTION == branch:
                self.successes.append("current")
                copy2(archive_name, CreateBinaryDistr.ARCHIVE_FILE_NAME % "current")

        # Clean again. Cleaning is good. You should clean your house more often too!
        CreateBinaryDistr._clean()

    @staticmethod
    def _clean_old_archives():
        files = glob("./PropWare_*.zip")
        for f in files:
            os.remove(f)

    @staticmethod
    def _clean():
        with open(os.devnull, 'w') as devnull:
            if CreateBinaryDistr._is_cmake_branch():
                build_dir = CreateBinaryDistr.PROPWARE_ROOT + str(os.sep) + 'bin'
                if os.path.exists(build_dir):
                    # Try to generate the Make files so that we can clean stuff up...
                    subprocess.call(['cmake', CreateBinaryDistr.PROPWARE_ROOT], stdout=devnull, stderr=devnull,
                                    cwd=build_dir)
            else:
                build_dir = CreateBinaryDistr.PROPWARE_ROOT

            if os.path.exists(build_dir):
                subprocess.call(["make", "clean", "--silent"], stderr=devnull, cwd=build_dir)
                sys.stdout.flush()

                # Not all branches have the simple_clean target, so it's no big deal if it fails
                try:
                    subprocess.check_output(["make", "simple_clean", "--silent"], stderr=devnull, cwd=build_dir)
                except subprocess.CalledProcessError as e:
                    if CreateBinaryDistr.MAKE_CLEAN_FAILED_CODE != e.returncode:
                        raise e

    @staticmethod
    def _clean_untracked():
        # If we've made it this far without failure, then clean all untracked files (leftovers from the previous branch)
        subprocess.call(["git", "clean", "-fxd", "-ePropWare*.zip", "-e*.pyc"], cwd=CreateBinaryDistr.PROPWARE_ROOT)

    @staticmethod
    def _checkout(branch):
        assert (isinstance(branch, str))

        try:
            CreateBinaryDistr._clean_untracked()
            sys.stdout.flush()
            subprocess.check_output(["git", "checkout", branch])
        except subprocess.CalledProcessError:
            print("Failed to checkout " + branch, file=sys.stderr)
            return 1

        if branch not in CreateBinaryDistr.TAGS:
            try:
                subprocess.check_output(["git", "pull"])
            except subprocess.CalledProcessError:
                print("Failed to pull latest sources", file=sys.stderr)
                return 1

        sys.stdout.flush()
        print("Now in branch: " + branch)
        sys.stdout.flush()

        return 0

    @staticmethod
    def _compile():
        # Determine if Makefile or CMake branch
        if CreateBinaryDistr._is_cmake_branch():
            build_dir = CreateBinaryDistr.PROPWARE_ROOT + str(os.sep) + 'bin'
            if not os.path.exists(build_dir):
                os.makedirs(build_dir)
            if 0 != subprocess.call(['cmake', CreateBinaryDistr.PROPWARE_ROOT], cwd=build_dir):
                sys.stdout.flush()
                raise MakeErrorException()
        else:
            build_dir = CreateBinaryDistr.PROPWARE_ROOT

        sys.stdout.flush()

        if 0 != subprocess.call(CreateBinaryDistr.MAKE_COMPILE, cwd=build_dir):
            sys.stdout.flush()
            raise MakeErrorException()

        sys.stdout.flush()

    @staticmethod
    def _is_whitelisted(filename):
        if filename in CreateBinaryDistr.WHITELISTED_FILES:
            return True
        else:
            filename = filename.split('.')
            if 2 == len(filename) and 0 != len(filename[0]):
                if filename[1].lower() in CreateBinaryDistr.WHITELIST_EXTENSIONS:
                    return True

        return False

    @staticmethod
    def _attempt_clean_exit():
        print("[INFO] !!!Attempting clean exit!!!")
        sys.stdout.flush()

        try:
            CreateBinaryDistr._clean_untracked()
            subprocess.check_output(["git", "checkout", CreateBinaryDistr.CURRENT_SUGGESTION])
        except subprocess.CalledProcessError as e:
            print("Failed to return git repository to 'current' branch", file=sys.stderr)
            print("Caused by: " + str(e), file=sys.stderr)
            print(e.output.decode(), file=sys.stderr)

    @staticmethod
    def _is_cmake_branch():
        return os.path.exists(CreateBinaryDistr.PROPWARE_ROOT + str(os.sep) + "CMakeLists.txt")

    @staticmethod
    def _is_branch_with_importer():
        return os.path.exists(
            CreateBinaryDistr.PROPWARE_ROOT + str(os.sep) + "util" + str(os.sep) + "propwareImporter.py")

    # noinspection PyShadowingNames
    def _print_summary(self, branches):
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


def parse_args():
    parser = argparse.ArgumentParser(description="Create binary distributions of all branches (and optionally tags too)"
                                                 " of PropWare")
    parser.add_argument("--tags", action="store_true",
                        help="Create binary distributions for all tagged commits as well")
    return parser.parse_args()


if "__main__" == __name__:
    args = parse_args()

    branches = CreateBinaryDistr.BRANCHES
    if args.tags:
        branches += CreateBinaryDistr.TAGS

    runMe = CreateBinaryDistr()
    runMe.run(branches)
