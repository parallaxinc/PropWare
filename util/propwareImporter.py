#/usr/bin/python
# File:    ${file}
# Author:  David Zemon
# Project: PropWare
#
# Created with: PyCharm Community Edition

"""
@description:
"""
__author__ = 'david'

from importLibpropeller import ImportLibpropeller
from importSimple import ImportSimple

# Import libpropeller
libpropellerImporter = ImportLibpropeller()
libpropellerImporter.run()

#import simple libraries
simpleImporter = ImportSimple()
simpleImporter.run()
