#!/usr/bin/python
# @file    propwareImporter.py
# @author  David Zemon
#
# Created with: PyCharm Community Edition

"""
@description:
"""
__author__ = 'david'

import propwareUtils


# noinspection PyPep8Naming
def importAll():
    from importLibpropeller import ImportLibpropeller
    from importSimple import ImportSimple

    propwareUtils.check_proper_working_dir()

    # Import libpropeller
    libpropeller_importer = ImportLibpropeller()
    libpropeller_importer.run()

    #import simple libraries
    simple_importer = ImportSimple()
    simple_importer.run()

if "__main__" == __name__:
    importAll()
