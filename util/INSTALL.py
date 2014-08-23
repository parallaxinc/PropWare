#!/usr/bin/python
# @file    INSTALL.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
from abc import abstractmethod
import argparse
import multiprocessing
import os
from os.path import expanduser
import re
import sys
import tempfile
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


class MissingBashrcException(Exception):
    def __init__(self):
        super(MissingBashrcException, self).__init__('Cannot find ~/.bashrc. ~/.bashrc is expected to exist so that I '
                                                     'can add environment variables for you.')


class Installer(object):
    _PROPGCC_DIR_NAME = 'propgcc'
    _PROPWARE_ROOT = ''

    __DEFAULT_PROPGCC_PATH = expanduser('~')
    __DEFAULT_CMAKE_PATH = expanduser('~')

    @classmethod
    def create(cls, operating_system):
        """
        :rtype : Installer
        """
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
        self._cmake_download_url = ''
        self._propgcc_download_url = ''
        self._cmake_parent = ''
        self._propgcc_parent = ''
        self._cmake_zip_name = None
        self._propgcc_zip_name = None
        self._cmake_installed = False
        self._cmake_root_dir_name = ''  # Differs between each OS - must be set in children
        self._cmake_path = ''
        self._propgcc_path = ''
        self._add_cmake_to_path = False
        self._add_propgcc_to_path = False
        self._export_env_var = ''
        self.cmd_sep = ''

        propwareUtils.check_proper_working_dir()
        Installer._PROPWARE_ROOT = os.path.abspath('..')

        # Parse arguments
        args = Installer._parse_args()
        if args.propgcc_path:
            self._propgcc_path = args.propgcc_path
        else:
            self._propgcc_parent = Installer.__DEFAULT_PROPGCC_PATH
        if args.cmake_path:
            self._cmake_path = args.cmake_path
        else:
            self._cmake_parent = Installer.__DEFAULT_CMAKE_PATH

    def install(self):
        self._confirm_dependencies()

        self._add_to_path()

        self._import_propware()  # Download Simple and libpropeller

        self._build_binaries()

    @classmethod
    def _parse_args(cls):
        parser = argparse.ArgumentParser(description='')
        parser.add_argument('-p', '--propgcc_path', dest='propgcc_path', action='store', type=str,
                            help='Path to install PropGCC contents', required=False)
        parser.add_argument('-c', '--cmake_path', dest='cmake_path', action='store', type=str,
                            help='Path to install CMake contents', required=False)
        return parser.parse_args()

    @classmethod
    def _import_propware(cls):
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
            return False
        else:
            return True

    @abstractmethod
    def _warn_make_instructions(self):
        pass

    def _add_to_path(self):
        cmake_bin = self._cmake_path + str(os.sep) + 'bin'
        propgcc_bin = self._propgcc_path + str(os.sep) + 'bin'

        if self._add_cmake_to_path:
            self._set_env_var('PATH', cmake_bin)

        if self._add_propgcc_to_path:
            self._set_env_var('PATH', propgcc_bin)

    def _build_binaries(self):
        has_make = self._check_for_make()
        if has_make and self._cmake_installed:
            run_cmake = ['cmake', '-G', 'Unix Makefiles', '.']
            print(' '.join(run_cmake))
            if 0 != subprocess.call(run_cmake, cwd=Installer._PROPWARE_ROOT):
                raise CMakeFailedException()

            # Dependency scanning isn't perfect. Seems to fail with 8 threads, so let's limit it to 4
            cpu_count = 4 if multiprocessing.cpu_count() > 4 else multiprocessing.cpu_count()
            run_make = ['make', '-j%d' % cpu_count]
            print(' '.join(run_make))
            subprocess.call(run_make, cwd=Installer._PROPWARE_ROOT)
        else:
            if not has_make:
                missing_dependency = 'Make'
            elif not self._cmake_installed:
                missing_dependency = 'CMake'
            else:
                raise Exception('Oh that\'s no good. Please contact david@zemon.name and inform him that you are '
                                'reading this right now.')
            print('Could not build PropWare due to missing %s. Sorry. :\'(' % missing_dependency, file=sys.stderr)

    def _confirm_dependencies(self):
        # ##
        # Query user about downloads
        # ##

        existing_cmake_bin = propwareUtils.which('cmake')
        if existing_cmake_bin:
            if 2 >= self._check_cmake_version(existing_cmake_bin)[0]:
                print('An existing version of CMake (version 2.x or older) has been detected. PropWare requires CMake'
                      '3.x or higher - a new version will be installed.')
                download_new_cmake = True
            else:
                existing_cmake_bin = os.path.realpath(existing_cmake_bin)
                cmake_bin_dir = os.path.split(existing_cmake_bin)[0]
                self._cmake_path = os.path.abspath(cmake_bin_dir + str(os.sep) + '..')
                download_new_cmake = False
                self._cmake_installed = True
        else:
            download_new_cmake = True

        if download_new_cmake:
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

        # ##
        # Download any required dependencies
        # ##

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
                self._cmake_installed = True

        if not self._propgcc_path:
            try:
                os.makedirs(self._propgcc_parent, 0o644)
            except OSError:
                pass
            finally:
                self._propgcc_zip_name = self._download_propgcc()
                propwareUtils.extract(self._propgcc_zip_name, self._propgcc_parent)
                self._propgcc_path = self._propgcc_parent + str(os.sep) + self._PROPGCC_DIR_NAME

    @abstractmethod
    def _check_for_default_propgcc(self):
        pass

    @classmethod
    def _set_env_var(cls, key, value):
        assert (isinstance(key, str))
        assert (isinstance(value, str))

        # Don't overwrite the PATH variable!!!
        if 'PATH' == key.upper():
            os.environ[key] = value + os.pathsep + os.environ[key]
        else:
            os.environ[key] = value

    @classmethod
    @abstractmethod
    def _add_root_env_var(cls, key, value):
        pass

    @classmethod
    def _check_cmake_version(cls, cmake_exe):
        assert (isinstance(cmake_exe, str))

        version_output = subprocess.check_output([cmake_exe, '--version'])
        version_line = version_output.split('\n')[0]
        version_number = version_line.split()[2]
        digits = version_number.split('.')
        version_tuple = (int(digits[0]), int(digits[1]), int(digits[2]))
        return version_tuple


class NixInstaller(Installer):
    def __init__(self):
        super(NixInstaller, self).__init__()
        self._export_env_var = 'export'
        self.cmd_sep = ';'

    def _check_for_default_propgcc(self):
        super(NixInstaller, self)._check_for_default_propgcc()

        if os.path.exists("/opt/parallax/bin/propeller-elf-gcc"):
            return "/opt/parallax"
        else:
            return None

    @abstractmethod
    def _warn_make_instructions(self):
        super(NixInstaller, self)._warn_make_instructions()

    @classmethod
    def _set_env_var(cls, key, value):
        if os.environ['SHELL'].endswith('/bash'):
            super(NixInstaller, cls)._set_env_var(key, value)

            if not os.path.exists(os.path.expanduser('~') + str(os.sep) + '.bashrc'):
                raise MissingBashrcException()

            if 'PATH' == key:
                cmd = 'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (value, os.path.expanduser('~'))
            else:
                cmd = 'echo "\nexport %s=%s" >> %s/.bashrc' % (key, value, os.path.expanduser('~'))

            print(cmd)
            subprocess.check_output(cmd, shell=True)
        else:
            if 'PATH' == key:
                error_str = 'Unknown shell is used. It is recommended that you add %s to your PATH.' % value
            else:
                error_str = \
                    'Unknown shell is used. It is recommended that you add %s=%s to your environment.' % (key, value)
            print(error_str, file=sys.stderr)

    @classmethod
    @abstractmethod
    def _add_root_env_var(cls, key, value):
        super(NixInstaller, cls)._add_root_env_var(key, value)


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

    def _add_to_path(self):
        super(DebInstaller, self)._add_to_path()

        if 'PROPGCC_PREFIX' not in os.environ or 'PROPWARE_PATH' not in os.environ:
            print('Environment variables will now be configured for the root environment.')
            prompt = 'Press "enter" to continue or "no" to configure them yourself.\n>>> '
            usr_input = propwareUtils.get_user_input(prompt, 'no'.__eq__, prompt, None)
            if None == usr_input:
                self._add_root_env_var('PROPGCC_PREFIX', self._propgcc_path)
                self._add_root_env_var('PROPWARE_PATH', Installer._PROPWARE_ROOT)
            else:
                print('You have selected to configure the following environment variables for yourself:')
                if 'PROPGCC_PREFIX' not in os.environ:
                    print('\tPlease set PROPGCC_PREFIX to "%s"' % self._propgcc_path)
                if 'PROPWARE_PATH' not in os.environ:
                    print('\tPlease set PROPWARE_PATH to "%s"' % Installer._PROPWARE_ROOT)

    def _check_for_make(self):
        if None == propwareUtils.which('make'):
            cmd = ['sudo', 'apt-get', 'install', 'make']
            print(' '.join(cmd))
            subprocess.call(cmd)

        # Run the parent AFTER we attempt to install Make
        return super(DebInstaller, self)._check_for_make()

    @classmethod
    def _needs_libc6(cls):
        cmd = ['dpkg-query', '-l', 'libc6*']
        package_list = subprocess.check_output(cmd).split(bytes('\n'))
        matcher = re.compile('.*libc6(:|-)i386.*')
        for package in package_list:
            if matcher.match(package):
                return False

        return True

    @classmethod
    def _add_root_env_var(cls, key, value):
        super(DebInstaller, cls)._add_root_env_var(key, value)

        if key not in os.environ:
            env_cmd = 'echo "%s=%s" >> /etc/environment' % (key, value)
            print(env_cmd)
            subprocess.call(env_cmd, shell=True)


class MacInstaller(NixInstaller):
    def __init__(self):
        super(MacInstaller, self).__init__()
        self._cmake_download_url = 'http://www.cmake.org/files/v3.0/cmake-3.0.1-Darwin-universal.tar.gz'
        self._cmake_root_dir_name = 'cmake-3.0.1-Darwin64-universal'
        self._propgcc_download_url = 'http://david.zemon.name/downloads/PropGCC-osx_10.6.8_v1_0_0.tar.gz'

    def _warn_make_instructions(self):
        print('WARNING: Make was not detected on your system. You can install it by following these instructions:\n\t'
              'http://stackoverflow.com/a/6767528', file=sys.stderr)

    def _add_to_path(self):
        super(MacInstaller, self)._add_to_path()
        # TODO: Set Mac environment variables

    @classmethod
    def _add_root_env_var(cls, key, value):
        super(MacInstaller, cls)._add_root_env_var(key, value)

        # TODO: Don't be a jerk. Figure out how to add root environment variables to Macs
        print("Uh oh! You need to add a root environment variable and I haven't figured out how yet! You should "
              "Google that.... Sorry. You need: %s=%s" % (key, value))


class WinInstaller(Installer):
    def __init__(self):
        super(WinInstaller, self).__init__()
        self._cmake_download_url = 'http://www.cmake.org/files/v3.0/cmake-3.0.1-win32-x86.zip'
        self._cmake_root_dir_name = 'cmake-3.0.1-win32-x86'
        self._propgcc_download_url = 'http://david.zemon.name/downloads/PropGCC-win_v1_0_0.zip'
        self._export_env_var = 'set'
        self.cmd_sep = '&'

    def _warn_make_instructions(self):
        print('WARNING: Make was not detected on your system. Make is packaged with PropGCC so be sure to add '
              'PropGCC\'s bin folder to your system\'s PATH environment variable.', file=sys.stderr)

    @classmethod
    def _set_env_var(cls, key, value):
        super(WinInstaller, cls)._set_env_var(key, value)

        from winutils import set_environ_var

        set_environ_var(key, value)

    @classmethod
    def _add_root_env_var(cls, key, value):
        super(WinInstaller, cls)._add_root_env_var(key, value)
        # No need for root environment variables here. User vars will do just fine in Windoze
        cls._set_env_var(key, value)

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
        # called
        return True


if '__main__' == __name__:
    installer = Installer.create(propwareUtils.get_os())
    installer.install()
