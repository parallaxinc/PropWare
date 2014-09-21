Installation
============

Microsoft Windows
-----------------
1. Download PropWare to your computer. You may use [git](http://msysgit.github.io/) to download the source code or 
   you may download a [binary distribution](http://david.zemon.name/downloads/PropWare_Binaries/PropWare_current.zip).
2. Extract the contents to a directory without any spaces. For instance, `C:\Users\David\PropWare` would be an excellent
   choice. However, `C:\Documents and Settings\David` would be an awful choice. For Windows XP users, I am sorry. You
   may need to use a path such as `C:\PropWare`.
3. Download Python 2.7 or later. At the time of this writing,
   [Python 3.4.1](https://www.python.org/ftp/python/3.4.1/python-3.4.1.msi) is the latest Windows revision. Refer to the
   python.org [downloads page](https://www.python.org/downloads/windows/) for the complete list of downloads.
4. Run the Python installation file or extract its contents to your desired directory. When selecting which components
   to install, ensure you have `Add python.exe to Path` set to install to disk.
   ![Python installation window](python_install.png)
5. Run `INSTALL.py`:
   * Open the PropWare root directory in your file browser
   * Open the `util` directory within PropWare
   * While holding `shift`, right-click on in the file browser window (not on a file) and choose `Open command window
    here`
    ![Right-click menu to open command window](open_cmd_window.png)
   * Type `INSTALL.py` and press enter. You might see some questions pop up - just answer them as your heart desires. 
     Since I'm still very new to this, _please_ [let me know](mailto:david@zemon.name) if there is anything confusing
     about my wording.
6. You're all done! If any errors were reported, please [let me know](mailto:david@zemon.name) or 
   [post in the forums](http://forums.parallax.com/showthread.php/157005-FYI-PropWare-Complete-build-system-and-library-for-PropGCC)
   so that I may help you through it or fix a bug in the installation.

Linux
-----
Most, if not all, Linux distributions ship with a version of Python, so don't worry about that. Simply download PropWare
in any way that you like (Git is recommended, but a 
[binary distribution](http://david.zemon.name/downloads/PropWare_Binaries/) will work just fine) and execute the
INSTALL.py script. If you downloaded PropWare to `/home/david/PropWare`, you would open a terminal and type:

    cd /home/david/PropWare/util
    python INSTALL.py

You might see some questions pop up - just answer them as your heart desires. Since I'm still very new to this, _please_
[let me know](mailto:david@zemon.name) if there is anything confusing about my wording.

When you're all done, exit the terminal. If you opted to set root environment variables, you'll need to reboot your 
computer before PropWare is usable.

Mac OSX
-------
I have _no_ idea what will happen on a Mac. I don't own a Mac. If you are reading this and you have a Mac, I'd greatly
appreciate you [getting in contact with me](mailto:david@zemon.name) to help me iron out any existing bugs in the 
installation script. You don't have to have any prerequisite knowledge to help - I just need something to test on.
