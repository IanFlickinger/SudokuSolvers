#include "solvers.h"
#include "puzzle.h"
#include <cmath>

#ifdef DEBUG_ANNEALING_SOLVERS_CPP
 #include <iostream>
 #include <iomanip>
#endif

using namespace Solvers;

AnnealingSolver::AnnealingSolver(unsigned iterations, unsigned reheats) {
    this->reheats = reheats;
    this->iterations = iterations;

    srand(time(NULL));
}

void AnnealingSolver::solve(Puzzle &puzzle) {
    #ifdef DEBUG_ANNEALING_SOLVERS_CPP
     std::cout << "AnnealingSolver::solve(Puzzle &)\n";
    #endif
    // "markov chain" config
    unsigned chainLength = 0;
    for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++) chainLength += puzzle.isConcrete(cell);
    chainLength = chainLength * chainLength;
    unsigned iterations = chainLength * this->iterations;

    #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
     std::cout << std::setw(55) << std::setfill('=') << '\n';
     std::cout << "Preparing " << this->reheats << " heats of " << iterations << " iterations\n"
               << "Markov chain length: " << chainLength << '\n'
               << "Initializing empty cells...\n";
    #endif

    // initialization
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

    // annealing
    double temperature;
    for (unsigned heat = 0; heat < reheats && !puzzle.isSolved(); heat++) {

        for (unsigned it = 0; it < iterations; it++) {
            if (it % chainLength == 0) {
                if (puzzle.isSolved()) return;
                temperature = this->tempSchedule(it, temperature);
            }

            // identify random change
            unsigned char row = rand() % puzzle.getSize();
            unsigned rowOffset = row * puzzle.getSize();
            unsigned char col1 = rand() % puzzle.getSize();
            for (unsigned char ctr = 0; puzzle.isConcrete(rowOffset+col1) && ctr < puzzle.getSize(); ctr++) col1 = ++col1 % puzzle.getSize();
            unsigned char col2 = rand() % puzzle.getSize();
            for (unsigned char ctr = 0; puzzle.isConcrete(rowOffset+col2) && ctr < puzzle.getSize(); ctr++) col2 = ++col2 % puzzle.getSize();
            unsigned cell1 = rowOffset + col1;
            unsigned cell2 = rowOffset + col2;
            if (puzzle.isConcrete(cell1) || puzzle.isConcrete(cell2) || cell1 == cell2) continue;

            // calculate delta
            int delta = puzzle.numConflictsAt(cell1) + puzzle.numConflictsAt(cell2);
            unsigned char temp = puzzle.getValue(cell1);
            puzzle.setValue(cell1, puzzle.getValue(cell2));
            puzzle.setValue(cell2, temp);
            delta -= puzzle.numConflictsAt(cell1) + puzzle.numConflictsAt(cell2);

            // calculate probability of change
            double prob = exp(delta / temperature);

            // sample change
            #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
             std::cout << "Iteration " << it << ": Attempt to swap columns " << std::to_string(col1) 
                       << " and " << std::to_string(col2) << " of row " << std::to_string(row)
                       << " with delta " << delta;
            #endif
            if (static_cast<double>(rand()) / RAND_MAX > prob) {
                #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
                std::cout << " failed\n";
                #endif
                unsigned char temp = puzzle.getValue(cell1);
                puzzle.setValue(cell1, puzzle.getValue(cell2));
                puzzle.setValue(cell2, temp);
            }
            #ifdef DEBUG_ANNEALING_SOLVERS_CPP_VERBOSE
             std::cout << " succeeded\n";
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

GeometricAnnealingSolver::GeometricAnnealingSolver(unsigned its, unsigned heats, double tempInit, double tempFact) 
    : AnnealingSolver(its, heats) 
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