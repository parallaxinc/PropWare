Download {#Download}
========

SimpleIDE (Any Operating System)
--------------------------------

The following steps explain how to use PropWare's libraries - including PropWare, Simple, and libpropeller - in 
a SimpleIDE project. Steps 2 and onward must be repeated for every new project in SimpleIDE.

1. Download the [libraries and header files][1]. Extract the contents to a known location on your disk - this 
   location will be referred to as `<PROPWARE_PATH>` for throughout these steps.
2. Set SimpleIDE to "Project view":
   1. Open the "Tools" menu and choose "Properties"
   2. Open the "General" tab and ensure "View Mode" is checked. Press "OK".
   3. Open the "Tools" menu and:<br />
      If the first option is "Set Simple View", then ignore it and press anywhere to exit the menu.<br />
      If the first option is "Set Project View", then select the first option.
3. Ensure project source files end with `.cpp` not `.c`.
4. From the "Project" menu, choose "Add Include Path". Select `<PROPWARE_PATH>/include` and then press the "Open" 
   button.
5. From the "Project" menu, choose "Add Library Path". Select `<PROPWARE_PATH>/lib` and then press the "Open" button.
6. Project Options
   * "Compiler Type" = `C`  
7. Compiler
   * "Enable Pruning" = `True`
   * "Other Compiler Options" = `-std=gnu++0x -fno-rtti -fno-threadsafe-statics`
8. Linker
   * "Other Linker Options" = `-lPropWare -lLibpropeller -lSimple`

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
sudo mv PropWare /Applications
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
sudo mv /Applications/PropWare /Applications/PropWare.bak
sudo mv OSXCMake /Applications
sudo mv PropWare /Applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once you have confirmed that the new installation works as expected, remove the old one with the following two lines.
NOTE: A command that starts with `sudo rm -rf` has the potential to be _very_ dangerous! I assume no responsibility 
if you corrupt your system. Be careful.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.sh}
sudo rm -rf /Applications/pwcmake.app.bak
sudo rm -rf /Applications/PropWare.bak
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

[1]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic-propware.zip?guest=1
[2]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.exe?guest=1
[3]: http://www.computerhope.com/issues/ch000549.htm
[4]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.deb?guest=1
[5]: http://david.zemon.name:8111/repository/download/PropWare_Release20/.lastSuccessful/PropWare-2.0.0-Generic.rpm?guest=1
