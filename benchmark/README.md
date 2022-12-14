# Comparing Solving Algorithms

Although this directory is titled benchmarking, at this point the code only compares algorithms on identical tests. It is recommended that the Solvers::DepthFirstSolverV1 class be used for benchmarking. For more information on the various algorithms, see the [solvers README](../src/solvers/README.md) file in the src/solvers directory.

The benchmarking library has two macros and one function with the following headers
 - #define SET_LOADER(loader)
 - #define ADD_SOLVER(solver, name)
 - void RUN_BENCHMARKS(int, char**)

## Getting Started

To run the benchmarking program, a Puzzle Loader instance must be defined using the [`SET_LOADER`](#puzzle-loader) macro, and at least one solver must be defined using the [`ADD_SOLVER`](#solvers) macro. The [`RUN_BENCHMARKS`](#benchmarking-program) function should then be called with the command-line arguments provided. The provided [benchmark_main.cpp](./benchmark_main.cpp) file shows an example of this, and has all built-in solver classes listed - simply comment out any solvers that are not intended to be compared. Here is an example running the compiled benchmark_main.cpp file over 10 iterations of 100 puzzles.

    benchmark 100 10

**With Cmake**: To utilize the provided cmake files, simply run cmake with the default options. Then run the `benchmark.exe` file as described above.

**Without Cmake**: Compile all cpp files in the *src* directory, as well as the *benchmark* directory. Include all files in the *include* directory as well as the *benchmark* directory.

## Puzzle Loader

To run the comparison, a PuzzleLoader instance must be created using the `SET_LOADER` macro defined in *benchmarking.h*. To use the macro, simply create an r-value reference as the macro argument.

    SET_LOADER(PuzzleLoader("../9x9.csv", 1000000UL, 9, 0))

The first argument to the built-in loader ("../9x9.csv") is the location of the csv file which should be formatted properly. 
The second argument (1000000UL) is the dataset size - the number of puzzles described in the data file.
The third argument (9) is the rank of the puzzle, where the classic 9x9 Sudoku is of rank 9.
The fourth argument is optional. When set to 0, the loader will start at the first puzzle, progress incrementally. If the fourth argument is not provided, or is not zero, the loader will select puzzles at random.

## Solvers

To add a solver, the process is similar to setting the puzzle loader. Simply call the provided `ADD_SOLVER` macro defined in *benchmarking.h* and pass an r-value reference to a solver object along with an identifiable name for the solver (this is for user reference in the output).

    ADD_SOLVER(Solvers::DepthFirstSolverV1(), Benchmark)

In the example line above, the `DepthFirstSolverV1` class is instantiated and added to the list of solvers with the name `Benchmark`. Note that - because this is a macro - quotation marks should not be used around the name.

## Benchmarking program

After calling SET_LOADER and ADD_SOLVER somewhere in the cpp file, the main function `RUN_BENCHMARKS` should be called with the command-line arguments described in [Getting Started](#getting-started).

## Puzzle Datasets

While no puzzle datasets have been directly associated with this repository yet, two recommended csv files are available on Github and Kaggle:

 - **1 Million 4x4 Puzzles** (Github): [Black-Phoenix/4x4-Sudoku-Dataset](https://www.github.com/Black-Phoenix/4x4-Sudoku-Dataset)
 - **1 Million 9x9 Puzzles** (Kaggle): [bryanpark/sudoku](https://www.kaggle.com/datasets/bryanpark/sudoku)

 To utilize the built-in PuzzleLoader class, the provided Sudoku dataset csv files must have a header line. Further the only two acceptable columns are one which defines the sudoku puzzle and one which defines the solution. The one which defines the puzzle must come before the one which defines the solution.
 
 There is currently no support for any puzzle larger than rank nine. 