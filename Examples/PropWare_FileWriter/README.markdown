PropWare FatFileWriter Example
==============================

Make a copy of a file.

Two versions of the program exist - one that uses the PropWare namespace and another that uses the Simple library and
C standard library. Program size is reported by `propeller-load`, so it is only a rough estimate. The test file was 
25.9 kB. Time includes mounting and file open/close actions.

PropWare
--------

CMM Size: 17,328 bytes<br>
CMM Time: 14.276 seconds

LMM Size: 29,028 bytes<br>
LMM Time: 3.978 seconds

Simple + C Standard Library
---------------------------

CMM Size: 15,308 bytes<br>
CMM Time: 39.088 seconds

LMM Size: 25,352 bytes<br>
LMM Time: 36.401 seconds
