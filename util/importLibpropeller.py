#/usr/bin/python
# @file    importLibpropeller.py
# @author  David Zemon
# @project PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
import os
from shutil import copytree, rmtree
import subprocess

__author__ = 'david'


class ImportLibpropeller:
    PROPWARE_ROOT = "../"
    DESTINATION = PROPWARE_ROOT + "libpropeller/"
    LIBPROPELLER_ENV_VAR = "LIBPROPELLER_PATH"

    @staticmethod
    def run():
        libpropellerPath = os.environ[ImportLibpropeller.LIBPROPELLER_ENV_VAR]
        subprocess.Popen(["git", "pull"], cwd=libpropellerPath)
        rmtree(ImportLibpropeller.DESTINATION)
        copytree(libpropellerPath + "/libpropeller", ImportLibpropeller.DESTINATION)

if "__main__" == __name__:
    importer = ImportLibpropeller()
    importer.run()
