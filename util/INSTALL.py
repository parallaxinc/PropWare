#!/usr/bin/python
# @file    INSTALL.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
import argparse
import os
from os.path import expanduser
import sys
import tempfile
import shutil

import propwareUtils


def my_input(prompt):
    try:
        # noinspection PyUnresolvedReferences
        return raw_input(prompt)
    except NameError:
        return input(prompt)


class Installer:
    _PROPWARE_ROOT = ""

    __DEFAULT_PROPGCC_PATH = expanduser("~")
    __DEFAULT_CMAKE_PATH = expanduser("~")

    def __init__(self):
        super().__init__()
        self._cmake_download_url = ""
        self._propgcc_download_url = ""
        self._cmake_zip_name = None
        self._propgcc_zip_name = None

        propwareUtils.checkProperWorkingDirectory()
        Installer._PROPWARE_ROOT = os.path.abspath("..")

        # Parse arguments
        args = Installer._parse_args()
        if None != args.propgcc_path:
            self._propgcc_path = args.propgcc_path
        else:
            self._propgcc_path = Installer.__DEFAULT_PROPGCC_PATH
        if None != args.cmake_path:
            self._cmake_path = args.cmake_path
        else:
            self._cmake_path = Installer.__DEFAULT_CMAKE_PATH

        # Confirm configuration settings
        self._cmake_path = Installer._get_user_input("CMake will be installed to %s. Press enter to continue or type "
                                                     "another existing path to download to a new directory.\n>>> ",
                                                     "'%s' does not exist or is not a directory.",
                                                     Installer.__DEFAULT_CMAKE_PATH)
        self._propgcc_path = Installer._get_user_input("PropGCC will be installed to %s. Press enter to continue or "
                                                       "type another existing path to download to a new "
                                                       "directory.\n>>> ",
                                                       "'%s' does not exists or is not a directory.",
                                                       Installer.__DEFAULT_PROPGCC_PATH)

    def install(self):
        self._download_dependencies()  # CMake and PropGCC

        self._copy_cmake_files()  # Copy necessary language files (*COG* and eventually Spin)

        self._import_propware()  # Download Simple and libpropeller

        # Last but not least, check to ensure Make is installed
        self._check_for_make()

    @staticmethod
    def _parse_args():
        parser = argparse.ArgumentParser(description="")
        parser.add_argument("-p", "--propgcc_path", dest="propgcc_path", action="store", type=str,
                            help="Path to install PropGCC contents", required=False)
        parser.add_argument("-c", "--cmake_path", dest="cmake_path", action='store', type=str,
                            help="Path to install CMake contents", required=False)
        return parser.parse_args()

    @staticmethod
    def _get_user_input(prompt, errorPrompt, default):
        usrInput = my_input(prompt % default)
        if "" != usrInput:
            if os.path.isdir(usrInput):
                return usrInput
            else:
                if None != errorPrompt:
                    print(errorPrompt % usrInput, file=sys.stderr)
                return Installer._get_user_input(prompt, errorPrompt, default)
        else:
            return default

    @staticmethod
    def _import_propware():
        if not os.path.exists(Installer._PROPWARE_ROOT + str(os.sep) + propwareUtils.DOWNLOADS_DIRECTORY):
            from propwareImporter import importAll
            importAll()

    def _download_cmake(self):
        assert (self._cmake_download_url != "")
        return propwareUtils.downloadFile(self._cmake_download_url, tempfile.gettempdir())[0]

    def _download_propgcc(self):
        assert (self._cmake_download_url != "")
        return propwareUtils.downloadFile(self._propgcc_download_url, tempfile.gettempdir())[0]

    def _check_for_make(self):
        if None == propwareUtils.which("make"):
            self._warn_make_instructions()

    def _warn_make_instructions(self):
        pass

    def _download_dependencies(self):
        self._cmake_zip_name = self._download_cmake()
        self._propgcc_zip_name = self._download_propgcc()

        propwareUtils.extractZip(self._cmake_zip_name, self._cmake_path)
        propwareUtils.extractZip(self._propgcc_zip_name, self._propgcc_path)

    def _copy_cmake_files(self):
        for root, dir, files in os.walk(self._cmake_path + str(os.sep) + "CMakeModules"):
            pass


class NixInstaller(Installer):
    def __init__(self):
        super().__init__()


class DebInstaller(NixInstaller):
    def __init__(self):
        super().__init__()
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Linux-i386.tar.gz"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-linux_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by executing \"sudo apt-get install "
              "make\".", file=sys.stderr)


class MacInstaller(NixInstaller):
    def __init__(self):
        super().__init__()
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Darwin-universal.tar.gz"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by following these instructions:\n\t"
              "http://stackoverflow.com/a/6767528", file=sys.stderr)


class WinInstaller(Installer):
    def __init__(self):
        super().__init__()
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-win32-x86.zip"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-win_v1_0_0.zip"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. Make is packaged with PropGCC so be sure to add "
              "PropGCC's bin folder to your system's PATH environment variable.", file=sys.stderr)


if "__main__" == __name__:
    verStr = str(sys.version_info[0]) + '.' + str(sys.version_info[1])

    installer = None
    platform = sys.platform
    if "linux2" == platform:
        installer = DebInstaller()
    elif "darwin" == platform:
        installer = MacInstaller()
    elif "win32" == platform:
        installer = WinInstaller()
    else:
        print("Your system (" + platform + ") could not be recognized. Please report this message to david@zemon.name.",
              file=sys.stderr)
        exit(1)

    installer.install()
