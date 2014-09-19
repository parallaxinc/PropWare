#!/usr/bin/python
# @file    propwareUtils.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
import glob
import os
import re
import shutil
import subprocess
import tarfile
import zipfile
import sys
import struct
import time

__author__ = 'david'

DOWNLOADS_DIRECTORY = '.external_downloads' + os.sep
MEMORY_MODELS = ['cog', 'cmm', 'lmm', 'xmmc', 'xmm-single', 'xmm-split']
FILE_ATTRIBUTE_HIDDEN = 0x02


class OperatingSystem(object):
    def __init__(self, platform):
        self._platform = platform

    def __str__(self):
        return self._platform

    def __eq__(self, y):
        return str(self) == str(y)


class Windows(OperatingSystem):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return 'Windows'


class Nix(OperatingSystem):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return '*Nix'


class Linux(Nix):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return 'Linux'


class Mac(Nix):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return 'Mac'


def get_os():
    platform = sys.platform
    if platform in ['linux', 'linux2']:
        return Linux()
    elif 'darwin' == platform:
        return Mac()
    elif 'win32' == platform:
        return Windows()
    else:
        return OperatingSystem(platform)


def which(program):
    """
    Search the system PATH environment variable in an attempt to find the requested program
    :param program: Name of executable or path (can be relative) to executable
    :return: Absolute path of requested program
    """
    assert (isinstance(program, str))

    system_path = os.environ['PATH'].split(os.pathsep)
    for path in system_path:
        candidate = os.path.join(path, program)
        if os.path.isfile(candidate):
            return candidate

    if Windows() == get_os():
        for path in system_path:
            candidate = os.path.join(path, program) + '.exe'
            if os.path.isfile(candidate):
                return candidate

    return None


def find(name, path):
    for root, dirs, files in os.walk(path):
        if name in files:
            return os.path.join(root, name)

    # Try adding '.exe' to the end and run again
    if Windows() == get_os():
        return find(name + '.exe', path)


def check_proper_working_dir():
    if 'createBinaryDistr.py' not in os.listdir('.'):
        raise IncorrectStartingDirectoryException()


def is_propware_root(directory):
    assert (isinstance(directory, str))
    assert (os.path.isdir(directory))

    return {'PropWare', 'util', 'libpropeller', 'simple'}.issubset(os.listdir(directory))


def is_python3():
    return '3' == sys.version[0]


def init_downloads_folder(propware_root):
    assert (is_propware_root(propware_root))

    full_path = os.path.abspath(propware_root) + str(os.sep) + DOWNLOADS_DIRECTORY

    # Create the folder if it doesn't exist
    if not os.path.exists(full_path):
        os.mkdir(full_path)

    # If on Windows, set the hidden attribute
    if Windows() == get_os():
        win_dir_name = os.path.normpath(full_path)
        import ctypes

        ctypes.windll.kernel32.SetFileAttributesW.argtypes = (ctypes.c_wchar_p, ctypes.c_uint32)
        ret = ctypes.windll.kernel32.SetFileAttributesW(win_dir_name, FILE_ATTRIBUTE_HIDDEN)
        if 0 == ret:  # return code of zero indicates failure, raise Windows error
            raise ctypes.WinError()


def download_file(src, dst_dir):
    file_name = src.split('/')[-1]
    dst = os.path.abspath(dst_dir) + str(os.sep) + file_name

    # If the file already exists, don't re-download it
    if os.path.exists(dst):
        return dst, None

    if is_python3():
        # noinspection PyUnresolvedReferences
        from urllib.request import urlopen
    else:
        # noinspection PyUnresolvedReferences
        from urllib2 import urlopen

    u = urlopen(src)
    with open(dst, 'wb') as f:
        meta = u.info()
        file_size = int(meta.get('Content-Length'))
        print('Downloading: %s - Bytes: %s' % (file_name, file_size))

        file_size_dl = 0
        block_size = 8192
        while True:
            buf = u.read(block_size)
            if not buf:
                break

            file_size_dl += len(buf)
            f.write(buf)
            status = r'%10d  [%3.2f%%]' % (file_size_dl, file_size_dl * 100. / file_size)
            status += chr(8) * (len(status))
            sys.stdout.write(status)

        sys.stdout.write(os.linesep)

        return dst, meta


def copytree(src, dst):
    src = os.path.normpath(src)
    dst = os.path.normpath(dst)

    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            shutil.copytree(s, d)
        else:
            shutil.copy2(s, d)


def is_asm_file(f):
    try:
        extension = f.split('.')[1]
    except IndexError:
        return False

    return extension in ['S', 's']


def is_src_file(f):
    assert (isinstance(f, str))
    return re.match('.*(\.c|\.cpp|\.cxx|\.cc|\.s|\.dat|\.cogc|\.ecogc|\.spin)$', f, re.I)


def is_header_file(f):
    assert (isinstance(f, str))
    return re.match('.*(\.h|\.hpp)$', f, re.I)


def is_src_or_hdr_file(f):
    return is_src_file(f) or is_header_file(f)


def test_propgcc():
    """
    Determine if PropGCC is installed and in the users PATH
    """
    subprocess.check_output(['propeller-elf-gcc', '--version'])


def extract(f, dst):
    """
    Supports any tar file as well as zips
    :param f: Path to compressed file or compressed file object
    :param dst: Path where contents should be extracted
    :return:
    """

    if tarfile.is_tarfile(f):
        open_method = tarfile.open
    elif zipfile.is_zipfile(f):
        open_method = zipfile.ZipFile
    else:
        raise NotRecognizedCompressedFile(f)

    with open_method(f, mode='r') as f:
        f.extractall(dst)


def get_user_input(prompt, condition, error_prompt, default):
    """

    :param prompt: String prompting user for input - if `default` != None, then `default` will be inserted into
    `prompt` via the % formatter and `prompt` should contain '%s' somewhere within it
    :param condition: Condition to accept response and return value
    :param error_prompt: String to print upon invalid response
    :param default: Default value to return if user enters nothing
    :return: User's response
    :rtype : str
    """
    assert (None == prompt or isinstance(prompt, str))
    # assert isinstance(condition, )
    assert (None == error_prompt or isinstance(error_prompt, str))
    assert (None == default or isinstance(default, str))

    def my_input(inner_prompt):
        try:
            # The readline module isn't always installed. So, if you don't have readline, you don't get autocompletion.
            # Sorry!

            # noinspection PyUnresolvedReferences
            import readline

            def complete(text, state):
                return (glob.glob(text+'*')+[None])[state]

            readline.set_completer_delims(' \t\n;')
            readline.parse_and_bind('tab: complete')
            readline.set_completer(complete)
        except ImportError:
            pass

        try:
            # noinspection PyUnresolvedReferences
            return raw_input(inner_prompt)
        except NameError:
            return input(inner_prompt)

    if prompt:
        if None != default and '%s' in prompt:
            short_prompt = prompt % default
        else:
            short_prompt = prompt
    else:
        short_prompt = ''
    usr_input = my_input(short_prompt)
    if usr_input:
        if condition(usr_input):
            return usr_input
        else:
            if error_prompt:
                print(error_prompt % usr_input, file=sys.stderr)
                sys.stderr.flush()
                time.sleep(0.01)
            return get_user_input(prompt, condition, error_prompt, default)
    elif default:
        return default
    else:
        return get_user_input(prompt, condition, error_prompt, default)


def get_cmake_modules_path(cmake_root):
    modules_dir_from_src_dstr = cmake_root + str(os.sep) + 'Modules'
    modules_dir_from_bin_dstr = cmake_root + str(os.sep) + 'share' + str(os.sep) + 'cmake-3.0' + str(os.sep) + 'Modules'

    if os.path.exists(modules_dir_from_src_dstr):
        return modules_dir_from_src_dstr
    elif os.path.exists(modules_dir_from_bin_dstr):
        return modules_dir_from_bin_dstr
    else:
        raise CannotFindCMakeModulesPath()


def is_64_bit():
    return 64 == struct.calcsize('P') * 8


def is_symbolic_link_on_windows(file_name):
    if Windows() == get_os():
        with open(file_name, 'r') as test_file:
            for i, l in enumerate(test_file):
                pass
            # noinspection PyUnboundLocalVariable
            return 1 == i + 1


class Menu(object):
    def __init__(self, prompt):
        self._prompt = prompt
        self._options = []
        self._default = None

    def add_option(self, name, default=False):
        assert (isinstance(default, bool))

        self._options.append(name)

        if default:
            self._default = name

        return self._options.index(name)

    def prompt(self):
        """
        :return: Selected value from menu
        :rtype str
        """
        options_len = len(self._options)

        assert (options_len > 1)

        prompt = '%s\n%s\n>>> ' % (self._prompt, self._get_menu_str())
        condition = re.compile('[1-%d]' % options_len).match
        error_prompt = '%s is not valid. Please select a menu number 1-' + str(options_len) + '.'
        default = self._get_default_value()
        selection = get_user_input(prompt, condition, error_prompt, default)
        selection = int(selection) - 1
        return self._options[selection]

    def _get_default_value(self):
        if self._default:
            return str(self._options.index(self._default) + 1)
        else:
            return None

    def _get_menu_str(self):
        menu_str = ''
        for i in range(len(self._options)):
            option = self._options[i]
            menu_str += '\t%d. %s' % (i + 1, option)
            if option == self._default:
                menu_str += ' [default]'
            if i != len(self._options) - 1:
                menu_str += os.linesep

        return menu_str


class IncorrectStartingDirectoryException(Exception):
    def __init__(self, *args, **kwargs):
        super(IncorrectStartingDirectoryException, self).__init__(*args, **kwargs)

    def __str__(self):
        return 'Must be executed from within <propware root>/util'


class CannotFindCMakeModulesPath(Exception):
    def __init__(self, *args, **kwargs):
        super(CannotFindCMakeModulesPath, self).__init__(*args, **kwargs)


class NotRecognizedCompressedFile(Exception):
    def __init__(self, f, *args, **kwargs):
        super(NotRecognizedCompressedFile, self).__init__(*args, **kwargs)
        if isinstance(f, str):
            self._filename = f
        else:
            self._filename = None

    def __str_(self):
        if None == self._filename:
            return super(NotRecognizedCompressedFile, self).__str__()
        else:
            return '"%s" is not a recognized compressed file type.' % self._filename
