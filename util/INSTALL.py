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

os.environ['PATH'] = '/home/david/reusable/cmake-3.0.1-Linux-i386/bin:' + os.environ['PATH']


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
            self._propgcc_parent = args.propgcc_path
        else:
            self._propgcc_parent = Installer.__DEFAULT_PROPGCC_PATH
        if args.cmake_path:
            self._cmake_parent = args.cmake_path
        else:
            self._cmake_parent = Installer.__DEFAULT_CMAKE_PATH

    def install(self):
        self._confirm_dependencies()

        self._add_to_path()

        self._import_propware()  # Download Simple and libpropeller

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
            self.add_env_var('PATH', cmake_bin)

        if self._add_propgcc_to_path:
            self.add_env_var('PATH', propgcc_bin)

    def _build_binaries(self):
        has_make = self._check_for_make()
        if has_make and self._cmake_installed:
            run_cmake = ['cmake', '-G', 'Unix Makefiles', '.']
            print(' '.join(run_cmake))
            if 0 != subprocess.call(run_cmake, cwd=Installer._PROPWARE_ROOT):
                raise CMakeFailedException()

            run_make = ['make', '-j%d' % multiprocessing.cpu_count()]
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
            existing_cmake_bin = os.path.realpath(existing_cmake_bin)
            cmake_bin_dir = os.path.split(existing_cmake_bin)[0]
            self._cmake_path = os.path.abspath(cmake_bin_dir + str(os.sep) + '..')
            download_new_cmake = False
            self._cmake_installed = True
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

    @staticmethod
    def add_env_var(key, value):
        assert (isinstance(key, str))
        assert (isinstance(value, str))
        os.environ[key] = value

    @staticmethod
    @abstractmethod
    def _add_root_env_var(key, value):
        pass


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
        pass

    @staticmethod
    def add_env_var(key, value):
        if os.environ['SHELL'].endswith('/bash'):
            NixInstaller().add_env_var(key, value)

            if not os.path.exists(os.path.expanduser('~') + str(os.sep) + '.bashrc'):
                raise MissingBashrcException()

            if 'PATH' == key:
                cmd = ['sh', '-c', 'echo "\nexport PATH=%s:\\$PATH" >> %s/.bashrc' % (value, os.path.expanduser('~'))]
            else:
                cmd = ['sh', '-c', 'echo "\nexport %s=%s" >> %s/.bashrc' % (key, value, os.path.expanduser('~'))]

            print(' '.join(cmd))
            subprocess.check_output(cmd)
        else:
            if 'PATH' == key:
                error_str = 'Unknown shell is used. It is recommended that you add %s to your PATH.' % value
            else:
                error_str = 'Unknown shell is used. It is recommended that you add %s=%s to your environment.' \
                            % (key, value)
            print(error_str, file=sys.stderr)

    @staticmethod
    @abstractmethod
    def _add_root_env_var(key, value):
        pass


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
                DebInstaller._add_root_env_var('PROPGCC_PREFIX', self._propgcc_path)
                DebInstaller._add_root_env_var('PROPWARE_PATH', Installer._PROPWARE_ROOT)
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

    @staticmethod
    def _needs_libc6():
        cmd = ['dpkg-query', '-l', 'libc6*']
        package_list = subprocess.check_output(cmd).split(bytes('\n'))
        matcher = re.compile('.*libc6(:|-)i386.*')
        for package in package_list:
            if matcher.match(package):
                return False

        return True

    @staticmethod
    def _add_root_env_var(key, value):
        super(DebInstaller)._add_root_env_var(key, value)

        if key not in os.environ:
            env_cmd = 'echo "%s=%s" >> /etc/environment' % (key, value)
            cmd = ['sudo', 'sh', '-c', env_cmd]
            print(' '.join(cmd))
            subprocess.call(cmd)


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

    @staticmethod
    def _add_root_env_var(key, value):
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

    @staticmethod
    def add_env_var(key, value):
        super(WinInstaller, key).add_env_var(value)

        from winutils import set_environ_var

        set_environ_var(key, value)

    @staticmethod
    def _add_root_env_var(key, value):
        # No need for root environment variables here. User vars will do just fine in Windoze
        WinInstaller.add_env_var(key, value)

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
