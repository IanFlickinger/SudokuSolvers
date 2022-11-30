#ifndef SUDOKU_DEBUGGING_H
#define SUDOKU_DEBUGGING_H

#include <iostream>
#include <iomanip>

#define DEBUG_HEADER_WIDTH 55

#define GET_DEBUG_DEF(file_name) "DEBUG_" #file_name "_CPP"
#define GET_DEBUG_DEF_VERBOSE(file_name) GET_DEBUG_DEF(file_name) "_VERBOSE"

#endif // SUDOKU_DEBUGGING_H