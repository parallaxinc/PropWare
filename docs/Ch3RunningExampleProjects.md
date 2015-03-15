Chapter 3: Run an Example Project
=================================

[<< Chapter 2: CMake and Make Tutorial](http://david.zemon.name/PropWare/md_docs_Ch2CMakeTutorial.html)<br />
[>> Chapter 4: Create a PropWare Application](http://david.zemon.name/PropWare/md_docs_Ch4AppDeployment.html)

PropWare comes pre-packaged with lots of example projects ready-to-go! Some were written to demonstrate PropWare 
classes, others show how to implement familiar code from the Parallax Learn library in a PropWare application. Here's
the gist for you:

* PropWare's example projects are not meant to be modified in-place. If you wish to make any changes, you are best off
  copying the code into a new location (if you don't already have one, I recommend you create a special folder, 
  dedicated to Propeller coding projects).
* Running PropWare's Example projects requires installing PropWare. Be sure to follow the installation instructions 
  first.
* Once PropWare is installed, create a new folder called `bin` inside your preferred example project and navigate to it
  from the command line. I installed PropWare to `/home/david/software/PropWare`, so I'm going to open a terminal and 
  type
  ~~~~~~~~~~~~~{.sh}
  cd /home/david/software/PropWare/Examples/GettingStarted
  mkdir bin
  cd bin
  ~~~~~~~~~~~~~
  If you're on windows, the installation folder might look more like `C:\Users\David\software\PropWare` instead. In 
  which case, the first command would be `cd C:\Users\David\software\PropWare\Examples\GettingStarted`. 
* We can build the example project with two easy commands:
  ~~~~~~~~~~~~~~~~{.sh}
  cmake -G "Unix Makefiles" ..
  make
  ~~~~~~~~~~~~~~~~
* And finally, plug in your Propeller's USB cable and type one last command:
  ~~~~~~~~~~~~~~~~{.sh}
  make debug
  ~~~~~~~~~~~~~~~~
  This final command should program the Propeller's RAM and start a terminal to display any output from the board. The
  GettingStarted project is a great place to start - it does not require any specific board or connected hardware.

[<< Chapter 2: CMake and Make Tutorial](http://david.zemon.name/PropWare/md_docs_Ch2CMakeTutorial.html)<br />
[>> Chapter 4: Create a PropWare Application](http://david.zemon.name/PropWare/md_docs_Ch4AppDeployment.html)
