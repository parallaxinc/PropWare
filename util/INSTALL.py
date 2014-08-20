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
import re
import sys
import tempfile
import shutil
import subprocess
try:
    # noinspection PyUnresolvedReferences
    import grp
except ImportError:
    pass

try:
    # noinspection PyUnresolvedReferences
    from pip.backwardcompat import PermissionError
except ImportError:
    pass

import propwareUtils
from propwareUtils import OperatingSystem, Windows, Linux, Mac


class CMakeFailedException(Exception):
    pass


class Installer(object):
    _PROPGCC_DIR_NAME = 'propgcc'
    _PROPWARE_ROOT = ''

    __DEFAULT_PROPGCC_PATH = expanduser('~')
    __DEFAULT_CMAKE_PATH = expanduser('~')

    @staticmethod
    def create(operating_system):
        assert (isinstance(operating_system, OperatingSystem))

        if isinstance(operating_system, Linux):
            return DebInstaller()
        elif isinstance(operating_system, Mac):
            return MacInstaller()
        elif isinstance(operating_system, Windows):
            return WinInstaller()
        else:
            raise Exception('Your operating system (%s) is not supported. Please report this error to '
                            'david@zemon.name' % str(operating_system))

    def __init__(self):
        super(Installer, self).__init__()
        self._cmake_download_url = ''
        self._propgcc_download_url = ''
        self._cmake_parent = ''
        self._propgcc_parent = ''
        self._cmake_zip_name = None
        self._propgcc_zip_name = None
        self._make_installed = False
        self._cmake_installed = False
        self._cmake_root_dir_name = ''  # Differs between each OS - must be set in children
        self._cmake_path = ''
        self._propgcc_path = ''
        self._add_cmake_to_path = False
        self._add_propgcc_to_path = False
        self._export_env_var = ''

        propwareUtils.check_proper_working_dir()
        Installer._PROPWARE_ROOT = os.path.abspath('..')

        # Parse arguments
        args = Installer._parse_args()
        if args.propgcc_path:
            self._propgcc_parent = args.propgcc_path
        else:
            self._propgcc_parent = Installer.__DEFAULT_PROPGCC_PATH
        if args.cmake_path:
            self._cmake_parent = args.cmake_path
        else:
            self._cmake_parent = Installer.__DEFAULT_CMAKE_PATH

    def install(self):
        self._confirm_dependencies()

        self._copy_cmake_files()  # Copy necessary language files (*COG* and eventually Spin)

        self._set_env_variables()

        self._import_propware()  # Download Simple and libpropeller

        if self._make_installed and self._cmake_installed:
            self._build_binaries()

    @staticmethod
    def _parse_args():
        parser = argparse.ArgumentParser(description='')
        parser.add_argument('-p', '--propgcc_path', dest='propgcc_path', action='store', type=str,
                            help='Path to install PropGCC contents', required=False)
        parser.add_argument('-c', '--cmake_path', dest='cmake_path', action='store', type=str,
                            help='Path to install CMake contents', required=False)
        return parser.parse_args()

    @staticmethod
    def _import_propware():
        from propwareImporter import importAll

        importAll()

    def _download_cmake(self):
        assert (self._cmake_download_url != '')
        return propwareUtils.download_file(self._cmake_download_url, tempfile.gettempdir())[0]

    def _download_propgcc(self):
        assert (self._cmake_download_url != '')
        return propwareUtils.download_file(self._propgcc_download_url, tempfile.gettempdir())[0]

    def _check_for_make(self):
        if None == propwareUtils.which('make'):
            self._warn_make_instructions()
            self._make_installed = False
        else:
            self._make_installed = True

    def _warn_make_instructions(self):
        pass

    def _copy_cmake_files(self):
        cmake_modules_src_path = Installer._PROPWARE_ROOT + str(os.sep) + 'CMakeModules'
        cmake_modules_dst_path = propwareUtils.get_cmake_modules_path(self._cmake_path)

        try:
            for entry in os.listdir(cmake_modules_src_path):
                src_file_path = cmake_modules_src_path + str(os.sep) + entry
                if os.path.isfile(src_file_path):
                    shutil.copy(src_file_path, cmake_modules_dst_path)

            cmake_platform_src_path = cmake_modules_src_path + str(os.sep) + 'Platform'
            cmake_platform_dst_path = cmake_modules_dst_path + str(os.sep) + 'Platform'
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
        pass

    def _build_binaries(self):
        set_propgcc_prefix = '%s PROPGCC_PREFIX=%s' % (self._export_env_var, os.path.abspath(self._propgcc_path))
        set_propware_path = '%s PROPWARE_PATH=%s' % (self._export_env_var, os.path.abspath(self._PROPWARE_ROOT))
        run_cmake = self._cmake_path + str(os.sep) + 'bin' + str(os.sep) + 'cmake -G "Unix Makefiles" .'

        run_cmake_cmd = '%s ; %s ; %s' % (set_propgcc_prefix, set_propware_path, run_cmake)
        cmd = ['sh', '-c', run_cmake_cmd]

        print(run_cmake_cmd)
        if 0 != subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT):
            raise CMakeFailedException()

        run_make = 'make'
        run_make_cmd = '%s ; %s ; %s' % (set_propgcc_prefix, set_propware_path, run_make)
        cmd = ['sh', '-c', run_make_cmd]
        print(run_make_cmd)
        subprocess.call(cmd, cwd=Installer._PROPWARE_ROOT)

    def _confirm_dependencies(self):
        self._check_for_make()

        # ##
        # Query user about downloads
        ###

        existing_cmake_bin = propwareUtils.which('cmake')
        if existing_cmake_bin:
            existing_cmake_bin = os.path.realpath(existing_cmake_bin)
            cmake_bin_dir = os.path.split(existing_cmake_bin)[0]
            self._cmake_path = os.path.abspath(cmake_bin_dir + str(os.sep) + '..')
            if os.access(self._cmake_path, os.W_OK):
                download_new_cmake = False
            else:
                user_input = propwareUtils.get_user_input(
                    'CMake is installed to a write-protected directory. Administrative privileges will be required to '
                    'install PropWare. Would you like to download a new version of CMake or install to the existing '
                    'directory? (download/install) [default: %s]\n>>> ', re.compile('(download|install)', re.I).match,
                    '"%s" is invalid. Choose "download" to download a new version of CMake or "install" to install in '
                    'the existing directory (requires administrative privileges)\n>>> ', 'download')
                if 'install' == user_input.lower():
                    download_new_cmake = False
                else:
                    download_new_cmake = True
        else:
            download_new_cmake = True
            self._cmake_parent = propwareUtils.get_user_input(
                'CMake will be installed to %s. Press enter to continue or type another path to download to '
                'a new directory.\n>>> ', os.path.isdir, '"%s" does not exist or is not a directory.',
                self._cmake_parent)
            self._add_cmake_to_path = True

        existing_propgcc_bin = propwareUtils.which('propeller-elf-gcc')
        if existing_propgcc_bin:
            existing_propgcc_bin = os.path.realpath(existing_propgcc_bin)
            propgcc_bin_dir = os.path.split(existing_propgcc_bin)[0]
            self._propgcc_path = os.path.abspath(propgcc_bin_dir + str(os.sep) + '..')
        else:
            default_propgcc = self._check_for_default_propgcc()
            if default_propgcc:
                self._propgcc_path = default_propgcc
            else:
                self._propgcc_parent = propwareUtils.get_user_input(
                    'PropGCC will be installed to %s. Press enter to continue or type another existing path to '
                    'download '
                    'to a new directory.\n>>> ', os.path.isdir, '"%s" does not exists or is not a directory.',
                    self._propgcc_parent)
                self._add_propgcc_to_path = True

        ###
        # Download any required dependencies
        ###

        if download_new_cmake:
            try:
                os.makedirs(self._cmake_parent, 0o644)
            except OSError:
                # OSError most likely results from the directory already existing. If that is the case, just ignore it.
                pass
            finally:
                self._cmake_zip_name = self._download_cmake()
                propwareUtils.extract(self._cmake_zip_name, self._cmake_parent)
                self._cmake_path = self._cmake_parent + str(os.sep) + self._cmake_root_dir_name

        if not self._propgcc_path:
            try:
                os.makedirs(self._propgcc_parent, 0o644)
            except OSError:
                pass
            finally:
                self._propgcc_zip_name = self._download_propgcc()
                propwareUtils.extract(self._propgcc_zip_name, self._propgcc_parent)
                self._propgcc_path = self._propgcc_parent + str(os.sep) + self._PROPGCC_DIR_NAME

    def _check_for_default_propgcc(self):
        pass


class NixInstaller(Installer):
    def __init__(self):
        super(NixInstaller, self).__init__()
        self._export_env_var = 'export'

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        super(NixInstaller, self)._sudo_copy_cmake_files(cmake_modules_src_path, cmake_modules_dst_path)

        cmd = 'sudo cp -r ' + cmake_modules_src_path + ' ' + cmake_modules_dst_path
        print('Your CMake installation directory is write-protected. Please provide root level permissions (normally, '
              'your standard system password) to copy necessary files into CMake.')
        print(' '.join(cmd))
        if 0 == subprocess.call(cmd.split()):
            return True
        else:
            return False

    def _set_env_variables(self):
        super(NixInstaller, self)._set_env_variables()

        cmake_bin = self._cmake_path + str(os.sep) + 'bin'
        propgcc_bin = self._propgcc_path + str(os.sep) + 'bin'

        if os.environ['SHELL'].endswith('/bash'):
            if self._add_cmake_to_path:
                cmd = ['sh', '-c',
                       'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (cmake_bin, os.path.expanduser('~'))]
                print(' '.join(cmd))
                subprocess.check_output(cmd)

            if self._add_propgcc_to_path:
                cmd = ['sh', '-c',
                       'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (propgcc_bin, os.path.expanduser('~'))]
                print(' '.join(cmd))
                subprocess.check_output(cmd)
        else:
            print('Unknown shell is used. It is recommended that you add "%s" and "%s" to the PATH variable for your '
                  'user\'s environment.' % (cmake_bin, propgcc_bin))

    def _check_for_default_propgcc(self):
        super(NixInstaller, self)._check_for_default_propgcc()

        if os.path.exists("/opt/parallax/bin/propeller-elf-gcc"):
            return "/opt/parallax"
        else:
            return None


class DebInstaller(NixInstaller):
    def __init__(self):
        super(DebInstaller, self).__init__()

        self._cmake_download_url = 'http://www.cmake.org/files/v3.0/cmake-3.0.1-Linux-i386.tar.gz'
        self._cmake_root_dir_name = 'cmake-3.0.1-Linux-i386'
        self._propgcc_download_url = 'http://david.zemon.name/downloads/PropGCC-linux_v1_0_0.tar.gz'
        self._user_in_dialout = os.environ['USER'] in grp.getgrnam('dialout')[3]

    def _warn_make_instructions(self):
        print('WARNING: Make was not detected on your system. You can install it by executing "sudo apt-get install '
              'make".', file=sys.stderr)

    def _confirm_dependencies(self):
        super(DebInstaller, self)._confirm_dependencies()

        # 64-bit versions of Debian also need the 32-bit C libraries. Install them.
        if propwareUtils.is_64_bit():
            if DebInstaller._needs_libc6():
                cmd = ['sudo', 'dpkg', '--add-architecture', 'i386']
                print(' '.join(cmd))
                subprocess.call(cmd)

                cmd = ['sudo', 'apt-get', 'install', 'libc6:i386']
                print(' '.join(cmd))
                subprocess.call(cmd)

        # Also, add user to "dialout" group if necessary
        if not self._user_in_dialout:
            user_input = propwareUtils.get_user_input(
                'Your user must be added to the "dialout" group in order to program a Propeller chip. Root privileges '
                'are required. Should your user be added? [default: %s] (y/n)\n>>> ', re.compile('(y|n)', re.I).match,
                '"%s" is not valid. Please enter "y" or "n".\n>>> ', 'y')
            if user_input.lower() == 'y':
                cmd = ['sudo', 'usermod', '-a', '-G', 'dialout', os.environ['USER']]
                print(' '.join(cmd))
                subprocess.call(cmd)

    def _set_env_variables(self):
        super(DebInstaller, self)._set_env_variables()

        if 'PROPGCC_PREFIX' not in os.environ or 'PROPWARE_PATH' not in os.environ:
            print('Environment variables will now be configured for the root environment.')
            prompt = 'Press "enter" to continue or "no" to configure them yourself.\n>>> '
            usr_input = propwareUtils.get_user_input(prompt, 'no'.__eq__, prompt, None)
            if None == usr_input:
                if 'PROPGCC_PREFIX' not in os.environ:
                    propgcc_env = 'echo "PROPGCC_PREFIX=%s" >> /etc/environment' % self._propgcc_path
                    cmd = ['sudo', 'sh', '-c', propgcc_env]
                    print(' '.join(cmd))
                    subprocess.call(cmd)

                if 'PROPWARE_PATH' not in os.environ:
                    propware_env = 'echo "PROPWARE_PATH=%s" >> /etc/environment' % Installer._PROPWARE_ROOT
                    cmd = ['sudo', 'sh', '-c', propware_env]
                    print(' '.join(cmd))
                    subprocess.call(cmd)
            else:
                print('You have selected to configure the following environment variables for yourself:')
                if 'PROPGCC_PREFIX' not in os.environ:
                    print('\tPlease set PROPGCC_PREFIX to "%s"' % self._propgcc_path)
                if 'PROPWARE_PATH' not in os.environ:
                    print('\tPlease set PROPWARE_PATH to "%s"' % Installer._PROPWARE_ROOT)

    def _check_for_make(self):
        super(DebInstaller, self)._check_for_make()

        if None == propwareUtils.which('make'):
            cmd = ['sudo', 'apt-get', 'install', 'make']
            print(' '.join(cmd))
            subprocess.call(cmd)

        # Run the parent AFTER we attempt to install Make
        super(DebInstaller, self)._check_for_make()

    @staticmethod
    def _needs_libc6():
        cmd = ['dpkg-query', '-l', 'libc6*']
        package_list = subprocess.check_output(cmd).split('\n')
        matcher = re.compile('.*libc6(:|-)i386.*')
        for package in package_list:
            if matcher.match(package):
                return False

        return True


class MacInstaller(NixInstaller):
    def __init__(self):
        super(MacInstaller, self).__init__()
        self._cmake_download_url = 'http://www.cmake.org/files/v3.0/cmake-3.0.1-Darwin-universal.tar.gz'
        self._cmake_root_dir_name = 'cmake-3.0.1-Darwin64-universal'
        self._propgcc_download_url = 'http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz'

    def _warn_make_instructions(self):
        print('WARNING: Make was not detected on your system. You can install it by following these instructions:\n\t'
              'http://stackoverflow.com/a/6767528', file=sys.stderr)

    def _set_env_variables(self):
        super(MacInstaller, self)._set_env_variables()
        # TODO: Set Mac environment variables


class WinInstaller(Installer):
    def __init__(self):
        super(WinInstaller, self).__init__()
        self._cmake_download_url = 'http://www.cmake.org/files/v3.0/cmake-3.0.1-win32-x86.zip'
        self._cmake_root_dir_name = 'cmake-3.0.1-win32-x86'
        self._propgcc_download_url = 'http://david.zemon.name/downloads/PropGCC-win_v1_0_0.zip'
        self._export_env_var = 'set'

    def _warn_make_instructions(self):
        print('WARNING: Make was not detected on your system. Make is packaged with PropGCC so be sure to add '
              'PropGCC\'s bin folder to your system\'s PATH environment variable.', file=sys.stderr)

    def _sudo_copy_cmake_files(self, cmake_modules_src_path, cmake_modules_dst_path):
        super(WinInstaller, self)._sudo_copy_cmake_files(cmake_modules_src_path, cmake_modules_dst_path)

        # TODO: Don't just print a warning, attempt to copy the CMake files

        print('Your CMake installation directory is in a write-protected directory. Please copy the contents of %s '
              'into %s before attempting to use CMake for the Propeller or use a version of CMake in a different '
              'directory.' % (cmake_modules_src_path, cmake_modules_dst_path), file=sys.stderr)
        return False

    def _set_env_variables(self):
        super(WinInstaller, self)._set_env_variables()

        from winutils import set_environ_var

        cmake_bin = self._cmake_path + str(os.sep) + 'bin'
        propgcc_bin = self._propgcc_path + str(os.sep) + 'bin'

        if self._add_cmake_to_path:
            set_environ_var('PATH', cmake_bin)
        if self._add_propgcc_to_path:
            set_environ_var('PATH', propgcc_bin)
        if 'PROPWARE_PATH' not in os.environ.keys():
            set_environ_var('PROPWARE_PATH', self._PROPWARE_ROOT)
        if 'PROPGCC_PREFIX' not in os.environ.keys():
            set_environ_var('PROPGCC_PREFIX', self._propgcc_path)

    def _check_for_default_propgcc(self):
        super(WinInstaller, self)._check_for_default_propgcc()

        if os.path.exists(r"C:\Program Files (x86)\SimpleIDE\propeller-gcc\bin\propeller-elf-gcc"):
            return os.path.abspath("C:\Program Files (x86)\SimpleIDE\propeller-gcc")
        elif os.path.exists(r"C:\Program Files\SimpleIDE\propeller-gcc\bin\propeller-elf-gcc"):
            return os.path.abspath("C:\Program Files\SimpleIDE\propeller-gcc")
        elif os.path.exists(r"C:\propgcc\bin\propeller-elf-gcc"):
            return os.path.abspath(r"C:\propgcc")
        else:
            return None

    def _check_for_make(self):
        # Make will be installed along with PropGCC, so no need to worry about this
        # NOTE: DO NOT call the super() method. We DO NOT want to do anything on a Windows machine when this method is
        #       called
        pass


if '__main__' == __name__:
    installer = Installer.create(propwareUtils.get_os())
    installer.install()

    print("\nCongratulations! PropWare has been installed successfully! If root environment variables were "
          "configured, you will need to reboot your computer. Otherwise, open a new terminal and have fun!")
