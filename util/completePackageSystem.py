# /usr/bin/python
# File:    createBinaryDistr_v2
# Author:  David Zemon
# Project: PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""

from __future__ import print_function
import argparse
import os
import shutil
import tempfile
import sys
import subprocess

import createBinaryDistr

PROPWARE_PATH = os.environ['PROPWARE_PATH']
REPO_URL = "https://github.com/SwimDude0614/PropWare.git"
BINARY_CREATOR_FILENAME = "createBinaryDistr.py"
BROKEN_TAGS = ["v2.0-beta1"]


def completePackager():
    global newestBinaryCreator
    args = parseArgs()

    tempdir = tempfile.gettempdir()
    newPropWarePath = tempdir + os.sep + "PropWare"
    newestBinaryCreator = createBinaryDistr.CreateBinaryDistr(newPropWarePath)
    fixSystemPath()

    os.environ['PROPWARE_PATH'] = newPropWarePath
    sys.path = [newPropWarePath + os.sep + "util"] + sys.path

    branches = newestBinaryCreator.BRANCHES
    if args.tags:
        branches += newestBinaryCreator.TAGS

    failList = []
    for branch in branches:
        checkoutNewRepo(tempdir, branch)
        os.chdir(newPropWarePath)

        binaryCreator = getBinaryCreator(branch, newPropWarePath)

        run(binaryCreator, branch)

        copyZipAndCleanDir(binaryCreator, newPropWarePath, branch, failList)

    printSummary(branches, failList)


def parseArgs():
    parser = argparse.ArgumentParser(description="Create binary distributions of all branches (and optionally tags too)"
                                                 " of PropWare")
    parser.add_argument("--tags", action="store_true",
                        help="Create binary distributions for all tagged commits as well")
    return parser.parse_args()


def fixSystemPath():
    originPath = sys.path[0]
    originPath = originPath[:-5]  # Truncate the "/util" from the end

    removeMe = []
    for path in sys.path:
        if originPath in path:
            removeMe.append(path)

    sys.path = [path for path in sys.path if path not in removeMe]


def checkoutNewRepo(parentDir, branch):
    os.chdir(parentDir)

    if os.path.exists("PropWare"):
        shutil.rmtree("PropWare")

    try:
        subprocess.check_output(["git", "clone", REPO_URL])
        os.chdir("PropWare")
        subprocess.check_output(["git", "checkout", branch])
    except subprocess.CalledProcessError:
        print("Failed to clone from GitHub", file=sys.stderr)
        return 1

    return 0


def getBinaryCreator(branch, newPropWarePath):
    import createBinaryDistr

    if hasCreationScript(newPropWarePath):
        os.chdir("util")
        loadImportScripts()
        binaryCreator = reloadModule(createBinaryDistr)
        binaryCreator = binaryCreator.CreateBinaryDistr()
        if branch in BROKEN_TAGS:
            bandageBrokenTag()
    else:
        binaryCreator = newestBinaryCreator

    return binaryCreator


def hasCreationScript(propwarePath):
    utilPackage = propwarePath + os.sep + "util"
    return os.path.exists(utilPackage) and os.path.exists(utilPackage + os.sep + BINARY_CREATOR_FILENAME)


def reloadModule(module):
    try:
        # noinspection PyUnboundLocalVariable,PyUnresolvedReferences
        return reload(module)
    except NameError:
        return reloadModule_py3(module)


def run(binaryCreator, branch):
    requiresSecondArg = False
    try:
        binaryCreator.runInBranch(branch)
    except TypeError:
        requiresSecondArg = True
    if requiresSecondArg:
        # noinspection PyArgumentList
        binaryCreator.runInBranch(branch, branch in newestBinaryCreator.TAGS)


def copyZipAndCleanDir(binaryCreator, newPropWarePath, branch, failList):
    newZipFile = binaryCreator.ARCHIVE_FILE_NAME % branch
    if os.path.exists(newZipFile):
        shutil.copy(newZipFile, PROPWARE_PATH)
    else:
        failList.append(branch)
    shutil.rmtree(newPropWarePath)


def reloadModule_py3(module):
    # noinspection PyUnresolvedReferences
    from importlib import reload

    return reload(module)


def printSummary(branches, failList):
    print("\n\nSummary:")
    for branch in branches:
        if branch not in failList:
            print("\tPASS: " + branch)
    for branch in failList:
        print("\tFAIL: " + branch)


def loadImportScripts():
    global importLibpropeller
    global importSimple
    global importAll

    import importLibpropeller
    import importSimple
    import propwareImporter

    importLibpropeller = reloadModule(importLibpropeller)
    importSimple = reloadModule(importSimple)
    propwareImporter = reloadModule(propwareImporter)
    importAll = propwareImporter.importAll


def bandageBrokenTag():
    os.chdir("util")
    importAll()
    os.chdir("..")


if "__main__" == __name__:
    completePackager()
