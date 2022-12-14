# Sudoku Solvers

In this repository is a library with [sudoku-solving algorithms](./src/solvers/), and a program for [benchmarking](./benchmark/) these algorithms. The sudoku-generating portion is currently under construction.

## Debugging

Each file is equipped with two levels of compiler-conditioned debugging. The first level will only print function calls, while the verbose option will print more detailed information throughout the function execution. To enable basic debugging, #define `DEBUG_ENABLED` before the `#include debugging.h` line. To enable verbose debugging, #define `DEBUG_ENABLED_VERBOSE` before the aforementioned include line. 