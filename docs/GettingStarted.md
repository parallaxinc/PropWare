Getting Started {#GettingStarted}
===============

Learn %PropWare
--------------

* Read about the [inspiration](@ref About) behind %PropWare and its goals going forward
* Search the [API documentation](usergroup0.xhtml) for classes and functions
* Browse example [projects](examples.xhtml)


Install the %PropWare Software
-----------------------------

Download and run the [installer](@ref Download) for Windows or Linux.


Say Hello
---------

@note The content for this example can also be found in the installation directory under `Examples/GettingStarted` and
      on this website [here](GettingStarted_8cpp-example.xhtml).
      
@note The command prompt is the fastest way to get up and running and requires the fewest dependencies. If you 
      prefer to use an integrated development environment (IDE) or graphical user interface (GUI), see 
      [Using an IDE](@ref UsingAnIDE).

1. Create a folder named `GettingStarted` which will hold your project files, such 
   `C:\Users\David\Documents\Propeller\GettingStarted`. Create two files in the directory, the first named 
   `CMakeLists.txt` and the second named `GettingStarted.cpp`. Into the first, copy the following configuration file:
   @include Examples/GettingStarted/CMakeLists.txt
   and into the second, copy the following source code:
   @include Examples/GettingStarted/GettingStarted.cpp
2. Open a command prompt (type `cmd` into the Start Menu) and run these commands
   ```cmd
   > cd C:\Users\David\Documents\Propeller\GettingStarted
   > mkdir bin
   > cd bin
   ```
   This will create a new folder named `bin` in your project.
3. Connect your Propeller and run the following two commands. If it is the first time you have connected the Propller
   to your computer, wait for any dialogue boxes to finish installing drivers.
   ```cmd
   > cmake -G "Unix Makefiles" ..
   > make debug
   ```
   This will compile your project, download it to the Propeller, and open a terminal for you to see the program output.
