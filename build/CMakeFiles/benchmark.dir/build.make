# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = "C:/Program Files/CMake/bin/cmake.exe"

# The command to remove a file.
RM = "C:/Program Files/CMake/bin/cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:/Users/ianfl/Documents/Projects/Sudoku

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:/Users/ianfl/Documents/Projects/Sudoku/build

# Include any dependencies generated for this target.
include CMakeFiles/benchmark.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/benchmark.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/benchmark.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/benchmark.dir/flags.make

CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj: CMakeFiles/benchmark.dir/flags.make
CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj: CMakeFiles/benchmark.dir/includes_CXX.rsp
CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj: ../benchmark/benchmark_main.cpp
CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj: CMakeFiles/benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/ianfl/Documents/Projects/Sudoku/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj -MF CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj.d -o CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj -c C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmark_main.cpp

CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.i"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmark_main.cpp > CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.i

CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.s"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmark_main.cpp -o CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.s

CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj: CMakeFiles/benchmark.dir/flags.make
CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj: CMakeFiles/benchmark.dir/includes_CXX.rsp
CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj: ../benchmark/benchmarking.cpp
CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj: CMakeFiles/benchmark.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/ianfl/Documents/Projects/Sudoku/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj -MF CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj.d -o CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj -c C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmarking.cpp

CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.i"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmarking.cpp > CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.i

CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.s"
	C:/cygwin64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/ianfl/Documents/Projects/Sudoku/benchmark/benchmarking.cpp -o CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.s

# Object files for target benchmark
benchmark_OBJECTS = \
"CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj" \
"CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj"

# External object files for target benchmark
benchmark_EXTERNAL_OBJECTS =

benchmark.exe: CMakeFiles/benchmark.dir/benchmark/benchmark_main.cpp.obj
benchmark.exe: CMakeFiles/benchmark.dir/benchmark/benchmarking.cpp.obj
benchmark.exe: CMakeFiles/benchmark.dir/build.make
benchmark.exe: src/libsudoku.a
benchmark.exe: CMakeFiles/benchmark.dir/linklibs.rsp
benchmark.exe: CMakeFiles/benchmark.dir/objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:/Users/ianfl/Documents/Projects/Sudoku/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable benchmark.exe"
	"C:/Program Files/CMake/bin/cmake.exe" -E rm -f CMakeFiles/benchmark.dir/objects.a
	C:/cygwin64/bin/ar.exe qc CMakeFiles/benchmark.dir/objects.a @CMakeFiles/benchmark.dir/objects1.rsp
	C:/cygwin64/bin/g++.exe -g -Wl,--whole-archive CMakeFiles/benchmark.dir/objects.a -Wl,--no-whole-archive -o benchmark.exe -Wl,--out-implib,libbenchmark.dll.a -Wl,--major-image-version,0,--minor-image-version,0 @CMakeFiles/benchmark.dir/linklibs.rsp

# Rule to build all files generated by this target.
CMakeFiles/benchmark.dir/build: benchmark.exe
.PHONY : CMakeFiles/benchmark.dir/build

CMakeFiles/benchmark.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/benchmark.dir/cmake_clean.cmake
.PHONY : CMakeFiles/benchmark.dir/clean

CMakeFiles/benchmark.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" C:/Users/ianfl/Documents/Projects/Sudoku C:/Users/ianfl/Documents/Projects/Sudoku C:/Users/ianfl/Documents/Projects/Sudoku/build C:/Users/ianfl/Documents/Projects/Sudoku/build C:/Users/ianfl/Documents/Projects/Sudoku/build/CMakeFiles/benchmark.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/benchmark.dir/depend

