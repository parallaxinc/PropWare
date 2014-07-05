#!/usr/bin/python
# File:    ${file}
# Author:  David Zemon
# Project: PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
__author__ = 'david'

import propwareUtils


def importAll():
    from importLibpropeller import ImportLibpropeller
    from importSimple import ImportSimple

    propwareUtils.checkProperWorkingDirectory()

    # Import libpropeller
    libpropellerImporter = ImportLibpropeller()
    libpropellerImporter.run()

    #import simple libraries
    simpleImporter = ImportSimple()
    simpleImporter.run()

if "__main__" == __name__:
    importAll()
