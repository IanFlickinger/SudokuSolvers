# Sudoku Solvers

In this repository is a library with [sudoku-solving algorithms](./src/solvers/README.md), and a program for benchmarking these algorithms. The sudoku-generating portion is currently under construction.

This README will introduce the basics of using this repository. More detailed information is provided in the other READMEs which are linked here when appropriate.


--------------------

## Getting Started (with cmake)

The cmake setup for this project includes the following options:
 - TEST: compiles the test code when set to ON
 - GENERATE: compiles the sudoku generation code (not yet functional) when set to ON
 - DEBUG: compiles the code with compiler-conditioned debugging, controlled by the `debug_defs` variable in the primary CMakeLists.txt file. See [debugging](#debugging) below for more information.

### Comparing Solvers

--------------------

## Getting Started (without cmake)

### Comparing Solvers

--------------------

## Debugging

Every object file contains compiler-conditioned debugging statements which can be used to trace and analyze the code. To control which object files are compiled with debugging statements, enter the [CMakeLists.txt](CMakeLists.txt) file in the root directory (this directory). All available debug definitions should be listed at the top in the `set` command for the `debug_defs` variable. 

There are two modes for debugging each file:
1. `DEBUG_FILE_NAME_CPP` defines the basic level of debugging for the object file identified by *file_name.cpp*. In this mode, the function signature of every function call will be printed to the console.
2. `DEBUG_FILE_NAME_CPP_VERBOSE` defines the verbose level of debugging for its associated object file (*file_name.cpp*). In this mode, the function header will be printed along with other updates throughout the function. It is especially recommended to redirect console output to a logfile when this mode is enabled for any file as the flow of console output can become quite large in magnitude. For example, any solver files output every "decision" they make so the logic can be tracked.