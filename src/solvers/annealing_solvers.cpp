#include "solvers.h"
#include "puzzle.h"
#include <cmath>

#ifdef DEBUG_ANNEALING_SOLVERS_CPP
 #include <iostream>
 #include <iomanip>
#endif

using namespace Solvers;

/****************************************************************************\
 * TODO
 * - Modular acceptance probability function
 * - Other cooling schedules
 * - Modular markov chain length
 * - Normalize cost function to make temperature settings more predictable
 *   > For example: in the exponential acceptance probability function, 
 *     P(t, Δ) = exp(Δ / t). If Δ is guaranteed in a certain distribution, 
 *     setting the temperature based on target acceptance rates is possible.
\****************************************************************************/

AnnealingSolver::AnnealingSolver(unsigned reheats, unsigned iterations) {
    this->reheats = reheats;
    this->iterations = iterations;

    srand(time(NULL));
}

void AnnealingSolver::solve(Puzzle &puzzle) {
    #ifdef DEBUG_ANNEALING_SOLVERS_CPP
     std::cout << "AnnealingSolver::solve(Puzzle &)\n";
    #endif
    // calculate the markov chain length
    unsigned chainLength = puzzle.getSizeSquared();
    for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++) chainLength -= puzzle.isConcrete(cell);
    chainLength = chainLength * chainLength;
    unsigned iterations = chainLength * this->iterations;

    #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
     std::cout << std::setw(55) << std::setfill('=') << '\n';
     std::cout << "Preparing " << this->reheats << " heats of " << iterations << " iterations\n"
               << "Markov chain length: " << chainLength << '\n'
               << "Initializing empty cells...\n";
    #endif

    // initialize the puzzle so each row has every value
    for (unsigned char row = 0; row < puzzle.getSize(); row++) {
        unsigned char val = 1;
        unsigned rowOffset = row * puzzle.getSize();
        for (unsigned cell = rowOffset; cell < rowOffset + puzzle.getSize(); cell++)
            if (!puzzle.isConcrete(cell)) {
                do {
                    puzzle.setValue(cell, val++);
                } while (puzzle.hasConflictInRow(row));
            }
    }

    // begin simulated annealing
    double temperature;
    for (unsigned heat = 0; heat < reheats && !puzzle.isSolved(); heat++) {
        // Perform one heating iteration
        for (unsigned it = 0; it < iterations; it++) {
            // progress through temperature schedule after each chain-length
            if (it % chainLength == 0) {
                if (puzzle.isSolved()) return;
                temperature = this->tempSchedule(it, temperature);
            }

            // sample random row
            unsigned char row = rand() % puzzle.getSize();
            unsigned rowOffset = row * puzzle.getSize();

            // sample cells within the identified row that were initially empty
            unsigned char col1 = rand() % puzzle.getSize();
            for (unsigned char ctr = 0; puzzle.isConcrete(rowOffset+col1) && ctr < puzzle.getSize(); ctr++) col1 = ++col1 % puzzle.getSize();
            unsigned char col2 = rand() % puzzle.getSize();
            for (unsigned char ctr = 0; puzzle.isConcrete(rowOffset+col2) && ctr < puzzle.getSize(); ctr++) col2 = ++col2 % puzzle.getSize();
            unsigned cell1 = rowOffset + col1;
            unsigned cell2 = rowOffset + col2;

            // do not attempt swap if either cell was not initially empty, or the same cell was sampled twice
            if (puzzle.isConcrete(cell1) || puzzle.isConcrete(cell2) || cell1 == cell2) continue;

            // calculate change in conflicts
            int delta = puzzle.numConflictsAt(cell1) + puzzle.numConflictsAt(cell2);
            unsigned char temp = puzzle.getValue(cell1);
            puzzle.setValue(cell1, puzzle.getValue(cell2));
            puzzle.setValue(cell2, temp);
            delta -= puzzle.numConflictsAt(cell1) + puzzle.numConflictsAt(cell2);

            // calculate acceptance probability
            double prob = exp(delta / temperature);

            // sample acceptance probability
            #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
             std::cout << "Iteration " << it << ": Attempt to swap columns " << std::to_string(col1) 
                       << " and " << std::to_string(col2) << " of row " << std::to_string(row)
                       << " with delta " << delta;
            #endif
            if (static_cast<double>(rand()) / RAND_MAX > prob) {
                // undo cell swap if change was not accepted
                #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
                std::cout << " failed\n";
                #endif
                unsigned char temp = puzzle.getValue(cell1);
                puzzle.setValue(cell1, puzzle.getValue(cell2));
                puzzle.setValue(cell2, temp);
            }
            #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
             else std::cout << " succeeded\n";
            #endif
        }
        #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
         std::cout << "Heat " << heat << ": final conflict count is " << puzzle.numConflicts() << "\n";
        #endif
    }
    #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
     std::cout << std::endl;
    #endif
}

GeometricAnnealingSolver::GeometricAnnealingSolver(unsigned heats, unsigned its, double tempInit, double tempFact) 
    : AnnealingSolver(heats, its) 
{
    this->tempInit = tempInit;
    this->tempFact = tempFact;
}

inline double GeometricAnnealingSolver::tempSchedule(unsigned iteration, double temperature) {
    #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
     double retVal = tempInit * (iteration == 0) + (iteration != 0) * tempFact * temperature;
     std::cout << "GeometricAnnealingSolver::tempSchedule(" << iteration << ", " << temperature << ") = " << retVal << '\n';
     return retVal;
    #elif defined DEBUG_ANNEALING_SOLVERS_CPP
     std::cout << "GeometricAnnealingSolver::tempSchedule(" << iteration << ", " << temperature << ")";
     return tempInit * (iteration == 0) + (iteration != 0) * tempFact * temperature;
    #else
     return tempInit * (iteration == 0) + (iteration != 0) * tempFact * temperature;
    #endif
}