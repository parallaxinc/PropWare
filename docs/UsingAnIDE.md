Using an IDE {#UsingAnIDE}
============

Ensure %PropWare is installed on your system prior to using an IDE.

JetBrain's CLion
----------------

1. Configure CLion to use %PropWare's CMake distribution instead of the version embedded in CLion:
   1. `File` -> `Settings...`
   2. `Build, Execution, Deployment` -> `Toolchains`
   3. For `CMake Executable`, choose "Use specified:" and then select `/usr/bin/cmake` for Linux or 
      `C:\%PropWare\PWCMake\bin\cmake.exe` for Windows.
2. Create your [CMakeLists.txt file](@ref BuildSystem).
3. Open CLion and choose `File` -> `Open...`. Navigate to your CMakeLists.txt file and press `Ok`.

Eclipse CDT (and other IDEs)
----------------------------

1. For Eclipse users, ensure the C/C++ Developer Tools (CDT) plugin is installed; For new Eclipse users, find
   `Eclipse IDE for C/C++ Developers` [here](http://www.eclipse.org/downloads/) and download the appropriate package
2. Create your [CMakeLists.txt file](@ref BuildSystem).
3. For any source file that does not yet exist, create an empty file in its place. CMake will throw errors if your
   CMakeLists.txt file references any non-existing source files.
4. At the terminal, enter your project's directory and type `cmake -G "Eclipse CDT4 - Unix Makefiles" .`. Notice that
   this time, we're adding a hook to tell CMake that we're using Eclipse. If you prefer another IDE, check the [CMake
   Wiki](http://www.cmake.org/Wiki/CMake_Generator_Specific_Information) to see if it is supported.
5. Navigating the Eclipse import dialogue:
  1. Open Eclipse. In the `File` menu, select `Import...`.
  2. Under the `General` group should be an option `Existing Projects into Workspace` - select that option and choose 
     `Next >`.
  3. On the next page, ensure `Select root directory:` is selected at the top and then click `Browse...`. Find the
     parent folder of your project (for example, if your project is `C:\Users\David\Hello`, then choose 
     `C:\Users\David`).
  4. At least one project should populate in the box below. Ensure your new %PropWare project and only your new
     %PropWare project is selected.
  5. Eclipse will import all necessary settings as soon as you click `Finish`. Compile flags, include directories and
     required source files will all be ready to go.
6. Compiling your program and loading to EEPROM can be done easily from within Eclipse. Compiling is as simple as 
   clicking the hammer icon or using `Control-B`. Writing to EEPROM requires creating a new Run Configuration.
  - I do not recommend attempting to create a Run Configuration for %PropWare's `debug` configuration. PropGCC's 
    terminal does not play well with either Eclipse or JetBrains CLion. Use the terminal for debugging, exactly as
    described in "Starting a New Project" step 4.
