#!/usr/bin/python
# @file    winutils
# @author  David Zemon
#
# Created with: PyCharm Community Edition

try:
    # noinspection PyUnresolvedReferences
    from _winreg import *
except ImportError:
    # noinspection PyUnresolvedReferences
    from winreg import *


def query_value(key, name):
    value, type_id = QueryValueEx(key, name)
    return value


def show(key):
    for i in range(1024):
        try:
            n, v, t = EnumValue(key, i)
            print('%s=%s' % (n, v))
        except EnvironmentError:
            break


def set_environ_var(name, value):
    assert (isinstance(name, str))
    assert (isinstance(value, str))

    key = None
    reg = None
    try:
        path = r'Environment'
        key = OpenKey(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS)  # Don't specify parameter name; it changed in Python3

        if name.upper() == 'PATH':
            try:
                value = value + ';' + query_value(key, name)
            except WindowsError as e:
                # Error #2 means the environment variable doesn't exist. For the PATH, that's no problem, we'll just
                # ignore it and create the variable immediately following.
                if 2 != e.errno:
                    raise e
        if value:
            SetValueEx(key, name, 0, REG_EXPAND_SZ, value)
    finally:
        if None != key:
            CloseKey(key)
        if None != reg:
            CloseKey(reg)


if '__main__' == __name__:
    print('Nothing to execute here! Program exiting...')