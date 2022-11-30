# set library files
set(lib_files
    puzzle.cpp
    data.cpp
    display.cpp
)
set(solver_files
    basic_solvers.cpp
    annealing_solvers.cpp
    graph_solvers.cpp
)

# add directory locations to files in subdirectories
foreach(solver_file ${solver_files})
    list(APPEND lib_files "solvers/${solver_file}")
endforeach()

# build library
add_library(sudoku ${lib_files})