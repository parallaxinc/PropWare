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


def complete_packager():
    global newestBinaryCreator
    args = parse_args()

    tempdir = tempfile.gettempdir()
    new_propware_path = tempdir + os.sep + "PropWare"
    newestBinaryCreator = createBinaryDistr.CreateBinaryDistr(new_propware_path)
    fix_system_path()

    os.environ['PROPWARE_PATH'] = new_propware_path
    sys.path = [new_propware_path + os.sep + "util"] + sys.path

    if None == args.branches:
        branches = newestBinaryCreator.BRANCHES
        if args.tags:
            branches += newestBinaryCreator.TAGS
    else:
        branches = args.branches

    fail_list = []
    for branch in branches:
        checkout_new_repo(tempdir, branch)
        os.chdir(new_propware_path)

        binary_creator = get_binary_creator(branch, new_propware_path)

        run(binary_creator, branch)

        copy_zip_and_clean_dir(binary_creator, new_propware_path, branch, fail_list)

    print_summary(branches, fail_list)


def parse_args():
    parser = argparse.ArgumentParser(description="Create binary distributions of all branches (and optionally tags too)"
                                                 " of PropWare")
    parser.add_argument("--tags", action="store_true",
                        help="Create binary distributions for all tagged commits as well")
    parser.add_argument("-b", "--branch", dest="branches", action='append')
    return parser.parse_args()


def fix_system_path():
    origin_path = sys.path[0]
    origin_path = origin_path[:-5]  # Truncate the "/util" from the end

    remove_me = []
    for path in sys.path:
        if origin_path in path:
            remove_me.append(path)

    sys.path = [path for path in sys.path if path not in remove_me]


def checkout_new_repo(parent_dir, branch):
    os.chdir(parent_dir)

    if os.path.exists("PropWare"):
        shutil.rmtree("PropWare")

    try:
        subprocess.check_output(["git", "clone", REPO_URL])
        os.chdir("PropWare")
        subprocess.check_output(["git", "_checkout", branch])
    except subprocess.CalledProcessError:
        print("Failed to clone from GitHub", file=sys.stderr)
        return 1

    return 0


def get_binary_creator(branch, new_propware_path):
    import createBinaryDistr

    if has_creation_script(new_propware_path):
        os.chdir("util")
        load_import_scripts()
        binary_creator = reload_module(createBinaryDistr)
        binary_creator = binary_creator.CreateBinaryDistr()
        if branch in BROKEN_TAGS:
            bandage_broken_tag()
    else:
        binary_creator = newestBinaryCreator

    return binary_creator


def has_creation_script(propware_path):
    util_package = propware_path + os.sep + "util"
    return os.path.exists(util_package) and os.path.exists(util_package + os.sep + BINARY_CREATOR_FILENAME)


def load_import_scripts():
    # noinspection PyGlobalUndefined
    global importAll

    sys.modules['importLibpropeller'] = reload_module(sys.modules['importLibpropeller'])
    sys.modules['importSimple'] = reload_module(sys.modules['importSimple'])
    sys.modules['propwareImporter'] = reload_module(sys.modules['propwareImporter'])
    # noinspection PyUnresolvedReferences
    from propwareImporter import importAll


def reload_module(module):
    if not module or not isinstance(module, types.ModuleType):
        raise TypeError("reload() argument must be module")

    try:
        # noinspection PyUnboundLocalVariable,PyUnresolvedReferences
        return reload(module)
    except NameError:
        import importlib
        # noinspection PyUnresolvedReferences
        return importlib.reload(module)


def run(binary_creator, branch):
    requires_second_arg = False
    try:
        binary_creator.runInBranch(branch)
    except TypeError:
        requires_second_arg = True
    if requires_second_arg:
        # noinspection PyArgumentList
        binary_creator.runInBranch(branch, branch in newestBinaryCreator.TAGS)


def copy_zip_and_clean_dir(binary_creator, new_propware_path, branch, fail_list):
    new_zip_file = binary_creator.ARCHIVE_FILE_NAME % branch
    if os.path.exists(new_zip_file):
        shutil.copy(new_zip_file, PROPWARE_PATH)
        if binary_creator.CURRENT_SUGGESTION == branch:
            shutil.copy(binary_creator.ARCHIVE_FILE_NAME % "current", PROPWARE_PATH)
    else:
        fail_list.append(branch)
    shutil.rmtree(new_propware_path)


def print_summary(branches, fail_list):
    print("\n\nSummary:")
    for branch in branches:
        if branch not in fail_list:
            print("\tPASS: " + branch)
    for branch in fail_list:
        print("\tFAIL: " + branch)


def bandage_broken_tag():
    os.chdir("util")
    importAll()
    os.chdir("..")


if "__main__" == __name__:
    complete_packager()
