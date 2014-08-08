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
import subprocess

import propwareUtils


class Installer:
    _PROPGCC_DIR_NAME = "propgcc"
    _CMAKE_DIR_NAME = "cmake-3.0"
    _PROPWARE_ROOT = ""

    __DEFAULT_PROPGCC_PATH = expanduser("~")
    __DEFAULT_CMAKE_PATH = expanduser("~")

    def __init__(self):
        self._cmake_download_url = ""
        self._propgcc_download_url = ""
        self._cmake_zip_name = None
        self._propgcc_zip_name = None
        self._make_installed = False
        self._cmake_installed = False

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
        self._cmake_path = propwareUtils.get_user_input("CMake will be installed to %s. Press enter to continue or "
                                                        "type "
                                                        "another existing path to download to a new directory.\n>>> ",
                                                        "'%s' does not exist or is not a directory.",
                                                        Installer.__DEFAULT_CMAKE_PATH + os.sep +
                                                        Installer._CMAKE_DIR_NAME)
        self._propgcc_path = propwareUtils.get_user_input("PropGCC will be installed to %s. Press enter to continue or "
                                                          "type another existing path to download to a new "
                                                          "directory.\n>>> ",
                                                          "'%s' does not exists or is not a directory.",
                                                          Installer.__DEFAULT_PROPGCC_PATH + os.sep +
                                                          Installer._PROPGCC_DIR_NAME)

    def install(self):
        self._download_dependencies()  # CMake and PropGCC

        self._copy_cmake_files()  # Copy necessary language files (*COG* and eventually Spin)

        self._import_propware()  # Download Simple and libpropeller

        self._set_env_variables()

        # Last but not least, check to ensure Make is installed
        self._check_for_make()

        if self._make_installed and self._cmake_installed:
            self._build_binaries()

    @staticmethod
    def _parse_args():
        parser = argparse.ArgumentParser(description="")
        parser.add_argument("-p", "--propgcc_path", dest="propgcc_path", action="store", type=str,
                            help="Path to install PropGCC contents", required=False)
        parser.add_argument("-c", "--cmake_path", dest="cmake_path", action='store', type=str,
                            help="Path to install CMake contents", required=False)
        return parser.parse_args()

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
            self._make_installed = False
        else:
            self._make_installed = True

    def _warn_make_instructions(self):
        pass

    def _download_dependencies(self):
        self._cmake_zip_name = self._download_cmake()
        self._propgcc_zip_name = self._download_propgcc()

        # TODO: THEY'RE NOT ZIPS YOU IDIOT!
        propwareUtils.extractZip(self._cmake_zip_name, self._cmake_path)
        propwareUtils.extractZip(self._propgcc_zip_name, self._propgcc_path)

    def _copy_cmake_files(self):
        cmake_modules_src_path = Installer._PROPWARE_ROOT + str(os.sep) + "CMakeModules"
        cmake_modules_dst_path = propwareUtils.get_cmake_modules_path(self._cmake_path)

        # noinspection PyUnresolvedReferences
        try:
            for entry in os.listdir(cmake_modules_src_path):
                src_file_path = cmake_modules_src_path + str(os.sep) + entry
                if os.path.isfile(src_file_path):
                    shutil.copy(src_file_path, cmake_modules_dst_path)

            cmake_platform_src_path = cmake_modules_src_path + str(os.sep) + "Platform"
            cmake_platform_dst_path = cmake_modules_dst_path + str(os.sep) + "Platform"
            for entry in os.listdir(cmake_platform_src_path):
                src_file_path = cmake_platform_src_path + str(os.sep) + entry
                if os.path.isfile(src_file_path):
                    shutil.copy(src_file_path, cmake_platform_dst_path)
        except (PermissionError, IOError):
            self._cmake_installed = self._sudo_copy_cmake_files(os.path.abspath(cmake_modules_src_path),
                                                                os.path.abspath(cmake_modules_dst_path))

        self._cmake_installed = True

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        assert (isinstance(cmake_modules_src_path, str))
        assert (isinstance(cmake_modules_dst_path, str))

        return False

    def _set_env_variables(self):
        # TODO
        pass

    def _build_binaries(self):
        pass


class NixInstaller(Installer):
    def __init__(self):
        Installer.__init__(self)

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        Installer._sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path)

        cmd = "sudo cp -r " + cmake_modules_src_path + ' ' + cmake_modules_dst_path
        print("Your CMake installation directory is write-protected. Please provide root level permissions (normally, "
              "your standard system password) to copy necessary files into CMake.")
        print(cmd)
        if 0 == subprocess.call(cmd.split()):
            return True
        else:
            return False

    def _build_binaries(self):
        Installer._build_binaries(self)

        cmd = [self._cmake_path + str(os.sep) + 'bin' + str(os.sep) + 'cmake', '-G', '"Unix Makefiles"', '.']
        if 0 != subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT):
            return

        cmd = ["make"]
        subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT)


class DebInstaller(NixInstaller):
    def __init__(self):
        NixInstaller.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Linux-i386.tar.gz"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-linux_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by executing \"sudo apt-get install "
              "make\".", file=sys.stderr)

    def _set_env_variables(self):
        Installer._set_env_variables(self)


class MacInstaller(NixInstaller):
    def __init__(self):
        NixInstaller.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-Darwin-universal.tar.gz"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. You can install it by following these instructions:\n\t"
              "http://stackoverflow.com/a/6767528", file=sys.stderr)


class WinInstaller(Installer):
    def __init__(self):
        Installer.__init__(self)
        self._cmake_download_url = "http://www.cmake.org/files/v3.0/cmake-3.0.1-win32-x86.zip"
        self._propgcc_download_url = "http://david.zemon.name/downloads/PropGCC-win_v1_0_0.zip"

    def _warn_make_instructions(self):
        print("WARNING: Make was not detected on your system. Make is packaged with PropGCC so be sure to add "
              "PropGCC's bin folder to your system's PATH environment variable.", file=sys.stderr)

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        Installer._sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path)
        print("Your CMake installation directory is in a write-protected directory. Please copy the contents of %s "
              "into %s before attempting to use CMake for the Propeller or use a version of CMake in a different "
              "directory." % (cmake_modules_src_path, cmake_modules_dst_path), file=sys.stderr)
        return False


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
        print("Your system (" + platform + ") could not be recognized. Please report this message to david@zemon.name",
              file=sys.stderr)
        exit(1)

    installer.install()
