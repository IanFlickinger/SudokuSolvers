#include <vector>

#include "solvers.h"
#include "puzzle.h"

#ifdef DEBUG_BASIC_SOLVERS_CPP
 #include <iostream>
 #include <iomanip>
#endif

void Solvers::DepthFirstSolverV1::solve(Puzzle &puzzle) {
    #ifdef DEBUG_BASIC_SOLVERS_CPP
     std::cout << "Solvers::DepthFirstSolverV1::solve(Puzzle&)\n";
    #endif
    #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
     std::cout << std::setw(55) << std::setfill('=') << "\n";
    #endif
    // Reset puzzle if provided with conflict
    if (puzzle.hasConflict()) puzzle.reset();

    // Initilize stacktracing vectors
    unsigned cursor = 0;
    std::vector<unsigned> cells;
    cells.reserve(puzzle.getSizeSquared());
    std::vector<unsigned> guesses;
    guesses.reserve(puzzle.getSizeSquared());

    // Search
    while (!puzzle.isSolved()) {
        #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
            std::cout << "Cursor at row " << cursor / puzzle.getSize() << " and column " << cursor % puzzle.getSize() << '\n';
        #endif
        if (puzzle.hasConflict()) {
            #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
             std::cout << "Attempting to resolve conflict...\n";
            #endif
            unsigned char guess = puzzle.getSize() + 1;
            while (guess > puzzle.getSize() && !guesses.empty()) {
                cursor = cells.back();
                cells.pop_back();
                guess = guesses.back() + 1;
                guesses.pop_back();
                // clear cell
                puzzle.setValue(cursor, 0);
            }
            #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
             std::cout << "Setting cell at row " << cursor / puzzle.getSize() << " and column " << cursor % puzzle.getSize() << " to " << std::to_string(guess) << '\n';
            #endif
            if (guess <= puzzle.getSize()) {
                puzzle.setValue(cursor, guess);
                cells.push_back(cursor);
                guesses.push_back(guess);
            }
        }
        else if (!puzzle.isConcrete(cursor)) {
            #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
             std::cout << "Setting cell at row " << cursor / puzzle.getSize() << " and column " 
                       << cursor % puzzle.getSize() << " to " << 1 << '\n';
            #endif
            puzzle.setValue(cursor, 1);
            cells.push_back(cursor);
            guesses.push_back(1);
        }
        else if (cursor + 1 >= puzzle.getSizeSquared()) 
            #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
            {
                std::cout << "ERROR: Cursor exceeded sudoku with neither conflict nor solution...\n";
                break;
            }
            #else
                break;
            #endif
        cursor++;
    }

    #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
     std::cout << std::endl;
    #endif
}

void Solvers::DepthFirstSolver::solve(Puzzle &puzzle) {
    #ifdef DEBUG_BASIC_SOLVERS_CPP
     std::cout << "Solvers::DepthFirstSolver::solve(Puzzle&)\n";
    #endif
    #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
     std::cout << std::setw(55) << std::setfill('=') << "\n";
    #endif
    // Initilize stacktracing vectors
    std::vector<unsigned> cells;
    cells.reserve(puzzle.getSizeSquared());
    std::vector<unsigned> guesses;
    guesses.reserve(puzzle.getSizeSquared());

    // Search
    unsigned node = 0;
    unsigned char guess = 1;
    while (node < puzzle.getSizeSquared()) {
        #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
            std::cout << "Cursor at row " << node / puzzle.getSize() << " and column " << node % puzzle.getSize() << '\n';
        #endif
        if (!puzzle.isConcrete(node)) {
            do {
                #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
                std::cout << "Setting cell at row " << node / puzzle.getSize() << " and column " 
                        << node % puzzle.getSize() << " to " << std::to_string(guess) << '\n';
                #endif
                puzzle.setValue(node, guess);
            } while (puzzle.hasConflictAt(node) && guess++ < puzzle.getSize());

            if (guess <= puzzle.getSize()) {
                cells.push_back(node++);
                guesses.push_back(guess);
                guess = 1;
            } else {
                while (guess > puzzle.getSize() && !guesses.empty()) {
                    // clear cell
                    #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
                    std::cout << "Clearing cell at row " << node / puzzle.getSize() 
                            << " and column " << node % puzzle.getSize() << '\n';
                    #endif
                    puzzle.setValue(node, 0);
                    // backtrack
                    node = cells.back();
                    cells.pop_back();
                    guess = guesses.back() + 1;
                    guesses.pop_back();
                }
            }
        } else node++;
    }

    #ifdef DEBUG_BASIC_SOLVERS_CPP_VERBOSE
     std::cout << std::endl;
    #endif
}