#include <vector>

#include "solvers.h"
#include "puzzle.h"

#define DEBUG_ENABLED false
#define DEBUG_ENABLED_VERBOSE false
#include "debugging.h"

void Solvers::DepthFirstSolverV1::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("Solvers::DepthFirstSolverV1::solve(Puzzle&)")

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
        DEBUG_OUTPUT("Cursor at row %d and column %d", cursor / puzzle.getSize(), cursor % puzzle.getSize())
        
        if (puzzle.hasConflict()) {
            DEBUG_OUTPUT("Attempting to resolve conflict...")
            unsigned char guess = puzzle.getSize() + 1;
            while (guess > puzzle.getSize() && !guesses.empty()) {
                cursor = cells.back();
                cells.pop_back();
                guess = guesses.back() + 1;
                guesses.pop_back();
                // clear cell
                puzzle.setValue(cursor, 0);
            }
            DEBUG_OUTPUT("Setting cell at row %d and column %d to %d", cursor / puzzle.getSize(), cursor % puzzle.getSize(), guess)
            // NOTE ON ABOVE: if guess is not printing properly, try immediately transforming to string using std::to_string
            if (guess <= puzzle.getSize()) {
                puzzle.setValue(cursor, guess);
                cells.push_back(cursor);
                guesses.push_back(guess);
            }
        }
        else if (!puzzle.isConcrete(cursor)) {
            DEBUG_OUTPUT("Setting cell at row %d and column %d to 1", cursor / puzzle.getSize(), cursor % puzzle.getSize())
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

    DEBUG_FUNC_END()
}

void Solvers::DepthFirstSolver::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("Solvers::DepthFirstSolver::solve(Puzzle&)")
    // Initilize stacktracing vectors
    std::vector<unsigned> cells;
    cells.reserve(puzzle.getSizeSquared());
    std::vector<unsigned> guesses;
    guesses.reserve(puzzle.getSizeSquared());

    // Search
    unsigned node = 0;
    unsigned char guess = 1;
    while (node < puzzle.getSizeSquared()) {
        DEBUG_OUTPUT("Cursor at row %d and column %d", node / puzzle.getSize(), node % puzzle.getSize())
        if (!puzzle.isConcrete(node)) {
            do {
                DEBUG_OUTPUT("Setting cell at row %d and column %d to %d", node / puzzle.getSize(), node % puzzle.getSize(), guess)
                puzzle.setValue(node, guess);
            } while (puzzle.hasConflictAt(node) && guess++ < puzzle.getSize());

            if (guess <= puzzle.getSize()) {
                cells.push_back(node++);
                guesses.push_back(guess);
                guess = 1;
            } else {
                while (guess > puzzle.getSize() && !guesses.empty()) {
                    // clear cell
                    DEBUG_OUTPUT("Clearing cell at row %d and column %d", node / puzzle.getSize(), node % puzzle.getSize())
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

    DEBUG_FUNC_END()
}