cmake_minimum_required(VERSION 3.5) # minimum specified in gtest build

# set debugging definitions
set(debug_defs 
    # DEBUG_BENCHMARK_CPP_VERBOSE
    # DEBUG_PUZZLE_CPP_VERBOSE
    # DEBUG_DATA_CPP_VERBOSE
    # DEBUG_DISPLAY_CPP
    # DEBUG_BASIC_SOLVERS_CPP_VERBOSE
    # DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
    # DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
    DEBUG_GENERATE_CPP
)

# test code
set(testCodeFiles
    test_puzzle.cpp
    test_solvers.cpp
    test_main.cpp
)

# compiler setup
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# visible options
option(TEST "Compiles the test code instead" OFF)
option(DEBUG "Enables debugging on all cpp files" OFF)
option(GENERATE "Enables sudoku generation" OFF)
set(options TEST DEBUG GENERATE)

# gtest library
set(GTEST_LOCATION C:/Users/ianfl/Documents/Projects/googletest)
set(GTEST_LOCATION_STATIC ${GTEST_LOCATION}/gtest_main.a)
set(GTEST_INCLUDE ${GTEST_LOCATION}/include)

# display options
foreach(opt ${options})
    message("${opt}: ${${opt}}")
endforeach()
message("")

# project settings
project(sudoku_solvers LANGUAGES CXX)
cmake_policy(SET CMP0079 NEW)

# build source library code
add_subdirectory(src)
target_include_directories(sudoku PUBLIC include include/solvers)

# add debug definitions when enabled
if(${DEBUG})
    # add nonverbose debugging for any verbose-enabled options
    foreach(debug_def ${debug_defs}) 
        if(debug_def MATCHES "_VERBOSE")
            string(REPLACE "_VERBOSE" "" other_def ${debug_def})
            list(APPEND debug_defs ${other_def})
        endif()
    endforeach(debug_def ${debug_defs})

    message("Debugging enabled")
    target_compile_definitions(sudoku PUBLIC ${debug_defs})
endif()

# branch to test build, or main build
if (TEST)
    # build test code    
    set(locatedTestFiles "")
    foreach(file ${testCodeFiles})
        list(APPEND locatedTestFiles "test/${file}")
    endforeach()
    add_executable(runTests ${locatedTestFiles})

    # include pre-built gtest library
    add_library(gtest STATIC IMPORTED GLOBAL)
    set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${GTEST_LOCATION_STATIC})

    # link test code to gtest and source libraries
    target_include_directories(runTests PRIVATE ${GTEST_INCLUDE} include)
    target_include_directories(runTests PRIVATE src)
    target_link_libraries(runTests gtest sudoku)
elseif(GENERATE)
    add_executable(generate generate/generate_main.cpp generate/generate.cpp)

    target_include_directories(generate PRIVATE include)
    target_link_libraries(generate sudoku)
else()
    # build benchmarking code
    add_executable(benchmark benchmark/benchmark_main.cpp benchmark/benchmarking.cpp)

    # link main code to source library
    target_include_directories(benchmark PRIVATE include)
    target_link_libraries(benchmark sudoku)
endif()
