Chapter 1: Installation
=======================

[>> Chapter 2: CMake and Make Tutorial][1]

Microsoft Windows
-----------------
1. Download [PropWare for Windows][2].
2. Execute the installer - make sure the installation path has no parentheses! In other words, _do not_ install PropWare
   into `C:\Program Files (x86)\...` - GCC won't like this one bit.
3. Add CMake's bin (located in `<install dir>\PWCMake\bin`) folder to your PATH. Excellent instructions for editing 
   PATH exist [here][3]. If the PATH variable already exists, add to it with a `;` separating the existing contents 
   and your new value. For instance, if you installed %PropWare to `C:\%PropWare` and PATH currently contains 
   `C:\propgcc\bin`, the new value would be `C:\%PropWare\PWCMake\bin;C:\propgcc\bin`.

Linux - Debian/Ubuntu/Mint/etc
------------------------------
1. Download both `.deb` files - one for the [PropWare headers and libraries][4], the other for [CMake][5].
2. Use `dpkg` to install both files. I use a single command to install both at once: `sudo dpkg -i PropWare*.deb`.

Linux - RedHat/Fedora/CentOS/etc
--------------------------------
1. Download both `.rpm` files - one for the [PropWare headers and libraries][6], the other for [CMake][7].
2. Use `rpm` to install both files. I use a single command to install both at once: `sudo rpm -i PropWare*.rpm`.

Mac OSX
-------
CMake supports multiple packaging systems for Mac, but I am unfamiliar with which one to use and have no way to 
test. Please let me know if you would like to help me package PropWare for Mac OSX.

[>> Chapter 2: CMake and Make Tutorial][5]

[1]: http://david.zemon.name/PropWare/md_docs_Ch2CMakeTutorial.html
[2]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.exe?guest=1
[3]: http://www.computerhope.com/issues/ch000549.htm
[4]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-propware.deb?guest=1
[5]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-cmake.deb?guest=1
[6]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-propware.rpm?guest=1
[7]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-cmake.rpm?guest=1
