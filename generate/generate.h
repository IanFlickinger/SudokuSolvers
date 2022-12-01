#ifndef SUDOKU_GENERATE_H
#define SUDOKU_GENERATE_H

#include "puzzle.h"
#include "solvers.h"

class SudokuGenerator {
    protected:
        const unsigned size;
        const unsigned ndims;
    public:
        SudokuGenerator(unsigned size, unsigned ndims) : 
            size(size), ndims(ndims) {};

        void fill(Puzzle *puzzles, unsigned num);
        virtual Puzzle build() = 0;
};

class MarkovAnnealingGenerator : SudokuGenerator {
    private:
        double resampleCap, alterCap;
        unsigned preheatIters;
        unsigned char *state;
        Solvers::AnnealingSolver * annealer;

        void sampleSolution();
        void alterSolution();
        Puzzle generatePuzzle();

    public:
        Puzzle build() override;

        MarkovAnnealingGenerator(unsigned size, unsigned ndims, double probResample, double probAlter, double probGenerate);
        ~MarkovAnnealingGenerator();
};

#endif // SUDOKU_GENERATE_H