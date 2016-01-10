Download {#Download}
========

Microsoft Windows
-----------------
1. Download [PropWare for Windows][2].
2. Execute the installer - make sure the installation path has no parentheses! In other words, _do not_ install 
   %PropWare into `C:\Program Files (x86)\...` - GCC won't like this one bit.
3. Add CMake's bin (located in `<install dir>\PWCMake\bin`) folder to your PATH. Excellent instructions for editing 
   PATH exist [here][3]. If the PATH variable already exists, add to it with a `;` separating the existing contents 
   and your new value. For instance, if you installed %PropWare to `C:\%PropWare` and PATH currently contains 
   `C:\propgcc\bin`, the new value would be `C:\%PropWare\PWCMake\bin;C:\propgcc\bin`.

Linux - Debian/Ubuntu/Mint/etc
------------------------------
1. Download the [`deb` package file][4].
2. Use `dpkg` to install the file with: `sudo dpkg -i PropWare-2.0.0-Generic.deb`.

Linux - RedHat/Fedora/CentOS/etc
--------------------------------
1. Download the [`rpm` package file][5].
2. Use `rpm` to install the file with: `sudo rpm -i PropWare-2.0.0-Generic.rpm`.

Mac OSX
-------
I am unable to produce easy installation packages for Mac. My apologies. Please execute the following lines at the 
command line to install %PropWare on your Mac:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-osx_cmake.zip?guest=1 -Ocmake.zip
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-propware.zip?guest=1 -Opropware.zip
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-examples.zip?guest=1 -Oexamples.zip
unzip cmake.zip
unzip propware.zip
unzip examples.zip
sudo mv pwcmake.app /Applications
sudo mv PropWare /usr/local
sudo ln -s /Applications/pwcmake.app/Contents/bin/cmake /usr/local/bin/cmake
sudo ln -s /Applications/pwcmake.app/Contents/bin/ccmake /usr/local/bin/ccmake
sudo ln -s /Applications/pwcmake.app/Contents/bin/cmake-gui /usr/local/bin/cmake-gui
sudo ln -s /Applications/pwcmake.app/Contents/bin/cpack /usr/local/bin/cpack
sudo ln -s /Applications/pwcmake.app/Contents/bin/ctest /usr/local/bin/ctest
sudo ln -s /Applications/pwcmake.app/Contents/bin/cmakexbuild /usr/local/bin/cmakexbuild
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To upgrade:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-osx_cmake.zip?guest=1 -Ocmake.zip
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-propware.zip?guest=1 -Opropware.zip
wget http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-examples.zip?guest=1 -Oexamples.zip
unzip cmake.zip
unzip propware.zip
unzip examples.zip
sudo mv /Applications/pwcmake.app /Applications/pwcmake.app.bak
sudo mv /usr/local/PropWare /usr/local/PropWare.bak
sudo mv OSXCMake /Applications
sudo mv PropWare /usr/local
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once you have confirmed that the new installation works as expected, remove the old one with the following two lines.
NOTE: A command that starts with `sudo rm -rf` has the potential to be _very_ dangerous! I assume no responsibility 
if you corrupt your system. Be careful.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
sudo rm -rf /Applications/pwcmake.app.bak
sudo rm -rf /usr/local/PropWare.bak
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[2]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.exe?guest=1
[3]: http://www.computerhope.com/issues/ch000549.htm
[4]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.deb?guest=1
[5]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.rpm?guest=1
