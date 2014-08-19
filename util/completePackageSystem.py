#!/usr/bin/python
# @file    completePackageSystem.py
# @author  David Zemon
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
import types

# noinspection PyUnresolvedReferences
import importLibpropeller
# noinspection PyUnresolvedReferences
import importSimple
# noinspection PyUnresolvedReferences
import propwareImporter
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

    if None == args.branches:
        branches = newestBinaryCreator.BRANCHES
        if args.tags:
            branches += newestBinaryCreator.TAGS
    else:
        branches = args.branches

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
    parser.add_argument("-b", "--branch", dest="branches", action='append')
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


def loadImportScripts():
    # noinspection PyGlobalUndefined
    global importAll

    sys.modules['importLibpropeller'] = reloadModule(sys.modules['importLibpropeller'])
    sys.modules['importSimple'] = reloadModule(sys.modules['importSimple'])
    sys.modules['propwareImporter'] = reloadModule(sys.modules['propwareImporter'])
    # noinspection PyUnresolvedReferences
    from propwareImporter import importAll


def reloadModule(module):
    if not module or not isinstance(module, types.ModuleType):
        raise TypeError("reload() argument must be module")

    try:
        # noinspection PyUnboundLocalVariable,PyUnresolvedReferences
        return reload(module)
    except NameError:
        import importlib
        return importlib.reload(module)


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
        if binaryCreator.CURRENT_SUGGESTION == branch:
            shutil.copy(binaryCreator.ARCHIVE_FILE_NAME % "current", PROPWARE_PATH)
    else:
        failList.append(branch)
    shutil.rmtree(newPropWarePath)


def printSummary(branches, failList):
    print("\n\nSummary:")
    for branch in branches:
        if branch not in failList:
            print("\tPASS: " + branch)
    for branch in failList:
        print("\tFAIL: " + branch)


def bandageBrokenTag():
    os.chdir("util")
    importAll()
    os.chdir("..")


if "__main__" == __name__:
    completePackager()
