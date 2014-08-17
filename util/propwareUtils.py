#!/usr/bin/python
# @file    propwareUtils.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
from __future__ import print_function
import os
import re
import shutil
import subprocess
import tarfile
import zipfile
import sys
import struct

__author__ = 'david'

DOWNLOADS_DIRECTORY = ".external_downloads" + os.sep
MEMORY_MODELS = ["cog", "cmm", "lmm", "xmmc", "xmm-single", "xmm-split"]


class OperatingSystem(object):
    def __init__(self, platform):
        self._platform = platform

    def __str__(self):
        return self._platform


class Windows(OperatingSystem):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return "Windows"


class Nix(OperatingSystem):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return "*Nix"


class Linux(Nix):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return "Linux"


class Mac(Nix):
    # noinspection PyMissingConstructor
    def __init__(self):
        pass

    def __str__(self):
        return "Mac"


def get_os():
    platform = sys.platform
    if platform in ["linux", "linux2"]:
        return Linux()
    elif "darwin" == platform:
        return Mac()
    elif "win32" == platform:
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

    def is_exe(filePath):
        if os.path.isfile(filePath) and os.access(filePath, os.X_OK):
            return True
        elif "nt" == os.name and ".exe" != filePath[-4:]:
            return is_exe(filePath + ".exe")
        else:
            return False

    directory, fileName = os.path.split(program)
    if directory:
        if is_exe(program):
            return os.path.abspath(program)
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return os.path.abspath(exe_file)

    if Windows() == get_os() and not program.endswith(".exe"):
        return which(program + ".exe")
    else:
        # If we haven't returned anything yet, that means the program doesn't exist
        return None


def checkProperWorkingDirectory():
    if "createBinaryDistr.py" not in os.listdir('.'):
        raise IncorrectStartingDirectoryException()


def isPropWareRoot(directory):
    assert (isinstance(directory, str))
    assert (os.path.isdir(directory))

    return "PropWare.dox" in os.listdir(directory)


def isPython3():
    return '3' == sys.version[0]


def initDownloadsFolder(propwareRoot):
    assert (isPropWareRoot(propwareRoot))

    fullPath = os.path.abspath(propwareRoot) + str(os.sep) + DOWNLOADS_DIRECTORY

    # Create the folder if it doesn't exist
    if not os.path.exists(fullPath):
        os.mkdir(fullPath)

    # If on Windows, set the hidden attribute
    if "nt" == os.name:
        winDirName = os.path.normpath(fullPath)
        import ctypes

        FILE_ATTRIBUTE_HIDDEN = 0x02

        ret = ctypes.windll.kernel32.SetFileAttributesW(winDirName, FILE_ATTRIBUTE_HIDDEN)
        if 0 == ret:  # return code of zero indicates failure, raise Windows error
            raise ctypes.WinError()


def downloadFile(src, dstDir):
    fileName = src.split('/')[-1]
    dst = os.path.abspath(dstDir) + str(os.sep) + fileName

    # If the file already exists, don't re-download it
    if os.path.exists(dst):
        return dst, None

    if isPython3():
        # noinspection PyUnresolvedReferences
        from urllib.request import urlopen
    else:
        # noinspection PyUnresolvedReferences
        from urllib2 import urlopen

    u = urlopen(src)
    with open(dst, 'wb') as f:
        meta = u.info()
        file_size = int(meta.get("Content-Length"))
        print("Downloading: %s - Bytes: %s" % (fileName, file_size))

        fileSizeDl = 0
        blockSize = 8192
        while True:
            buf = u.read(blockSize)
            if not buf:
                break

            fileSizeDl += len(buf)
            f.write(buf)
            status = r"%10d  [%3.2f%%]" % (fileSizeDl, fileSizeDl * 100. / file_size)
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


def isAsmFile(f):
    try:
        extension = f.split(".")[1]
    except IndexError:
        return False

    return extension in ["S", "s"]


def isSourceFile(f):
    assert (isinstance(f, str))
    return re.match('.*(\.c|\.cpp|\.cxx|\.cc|\.s|\.dat|\.cogc|\.ecogc|\.spin)$', f, re.I)


def isHeaderFile(f):
    assert (isinstance(f, str))
    return re.match('.*(\.h|\.hpp)$', f, re.I)


def isSourceOrHeaderFile(f):
    return isSourceFile(f) or isHeaderFile(f)


def test_propgcc():
    """
    Determine if PropGCC is installed and in the users PATH
    """
    subprocess.check_output(['propeller-elf-gcc', '--version'])


def extract(f, dest):
    """
    Supports any tar file as well as zips
    :param f: Path to compressed file or compressed file object
    :param dest: Path where contents should be extracted
    :return:
    """

    if tarfile.is_tarfile(f):
        open_method = tarfile.open
    elif zipfile.is_zipfile(f):
        open_method = zipfile.ZipFile
    else:
        raise NotRecognizedCompressedFile(f)

    with open_method(f, mode='r') as f:
        f.extractall(dest)


def get_user_input(prompt, condition, error_prompt, default):
    """
    :rtype : str
    """
    assert isinstance(prompt, str)
    # assert isinstance(condition, )
    assert (None == error_prompt or isinstance(error_prompt, str))
    assert (None == default or isinstance(default, str))

    def my_input(inner_prompt):
        try:
            # noinspection PyUnresolvedReferences
            return raw_input(inner_prompt)
        except NameError:
            return input(inner_prompt)

    if None != default:
        short_prompt = prompt % default
    else:
        short_prompt = prompt
    usr_input = my_input(short_prompt)
    if usr_input:
        if condition(usr_input):
            return usr_input
        else:
            if None != error_prompt:
                print(error_prompt % usr_input, file=sys.stderr)
            return get_user_input(prompt, condition, error_prompt, default)
    else:
        return default


def get_cmake_modules_path(cmake_root):
    modules_dir_from_src_dstr = cmake_root + str(os.sep) + "Modules"
    modules_dir_from_bin_dstr = cmake_root + str(os.sep) + "share" + str(os.sep) + "cmake-3.0" + str(os.sep) + "Modules"

    if os.path.exists(modules_dir_from_src_dstr):
        return modules_dir_from_src_dstr
    elif os.path.exists(modules_dir_from_bin_dstr):
        return modules_dir_from_bin_dstr
    else:
        raise CannotFindCMakeModulesPath()


def is_64_bit():
    return 64 == struct.calcsize('P') * 8


class IncorrectStartingDirectoryException(Exception):
    def __init__(self, *args, **kwargs):
        super(IncorrectStartingDirectoryException, self).__init__(*args, **kwargs)

    def __str__(self):
        return "Must be executed from within <propware root>/util"


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
            return "'%s' is not a recognized compressed file type." % self._filename
