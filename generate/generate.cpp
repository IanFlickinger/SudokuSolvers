// standard library
#include <stdlib.h>
#include <vector>

// sudoku library
#include "puzzle.h"
#include "solvers.h"
#include "display.h"

// generate library
#include "generate.h"

#ifdef DEBUG_GENERATE_CPP
 #include <iostream>
#endif

bool hasUniqueSolution(Puzzle &&puzzle) {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "hasUniqueSolution(Puzzle &&)" << std::endl;
    #endif
    // Initialize stacktracing vectors
    unsigned cell = 0, solutionCount = 0;
    std::vector<unsigned> cells;
    cells.reserve(puzzle.getSizeSquared());
    std::vector<unsigned> guesses;
    guesses.reserve(puzzle.getSizeSquared());

    // Search
    unsigned char guess = 1;
    // while (guess <= puzzle.getSize()) {
    while (true) {
        if (puzzle.hasConflict() || cell > puzzle.getSizeSquared()) {
            guess = puzzle.getSize() + 1;
            while (guess > puzzle.getSize() && !guesses.empty()) {
                cell = cells.back();
                cells.pop_back();
                guess = guesses.back() + 1;
                guesses.pop_back();
                // clear cell
                puzzle.setValue(cell, 0);
            }
            if (guess <= puzzle.getSize()) {
                puzzle.setValue(cell, guess);
                cells.push_back(cell);
                guesses.push_back(guess);
            } else return solutionCount == 1;
        }
        else if (!puzzle.isConcrete(cell)) {
            puzzle.setValue(cell, 1);
            cells.push_back(cell);
            guesses.push_back(1);
        }
        else if (cell + 1 >= puzzle.getSizeSquared()) solutionCount++;
        cell++;
    }
}

class PuzzleInProgress : public Puzzle {
    public:
        PuzzleInProgress(unsigned char size, unsigned char *values) {
            this->initializeSize(size);
            this->values = values;
        }
        bool isConcrete(unsigned cell) const { return false; }
		bool isConcrete(unsigned char row, unsigned char col) const 
			{ return false; }
};

void SudokuGenerator::fill(Puzzle *puzzles, unsigned num) {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "SudokuGenerator::fill(Puzzle *, " << num << ")" << std::endl;
    #endif
    for (Puzzle *cursor = puzzles, *max = cursor + num; cursor < max; cursor++)
        *cursor = this->build();
}

MarkovAnnealingGenerator::MarkovAnnealingGenerator(unsigned size, unsigned ndims, 
    double probResample, double probAlter, double probGenerate) : SudokuGenerator(size, ndims) 
{
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::MarkovAnnealingGenerator(" 
               << size << ", " << ndims << ", " << probResample << ", "
               << probAlter << ", " << probGenerate << ")" << std::endl;
    #endif
    // Initialize mdp probabilities
    double sum = probResample + probAlter + probGenerate;
    this->resampleCap = probResample / sum;
    this->alterCap = (probResample + probAlter) / sum;

    // Intialize annealer
    this->annealer = new Solvers::GeometricAnnealingSolver(1000, 1, 1000, 0.3);

    // Initialize puzzle state
    unsigned sizeSquared = size * size;
    this->state = new unsigned char[sizeSquared];
    this->preheatIters = size * perfectSqrt(size);
    for (unsigned char value = 1, *cell = this->state, *max = cell + sizeSquared; cell < max; cell++, value++) {
        if (value > size) value = 1;
        *cell = value;
    }
    this->sampleSolution();
}

MarkovAnnealingGenerator::~MarkovAnnealingGenerator() {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::~MarkovAnnealingGenerator()" << std::endl;
    #endif
    delete [] this->state;
    delete this->annealer;
}

Puzzle MarkovAnnealingGenerator::build() {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::build()" << std::endl;
    #endif
    while(true) {
        double r = std::rand() / static_cast<double>(RAND_MAX);

        if (r < this->resampleCap) this->sampleSolution();
        else if (r < this->alterCap) this->alterSolution();
        else return this->generatePuzzle();
    }
}

void MarkovAnnealingGenerator::sampleSolution() {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::sampleSolution()" << std::endl;
    #endif
    // Initial randomization
    unsigned char *values = this->state;
    for (unsigned iter = 0; iter < this->preheatIters; iter++) {
        unsigned char row = std::rand() % this->size;
        unsigned char col1 = std::rand() % this->size;
        unsigned char col2 = std::rand() % this->size;
        std::swap(this->state[COORDS_TO_CELL(row, col1, this->size)], this->state[COORDS_TO_CELL(row, col2, this->size)]);
    }
    // Anneal
    PuzzleInProgress puzzle(this->size, this->state);
    while(!puzzle.isSolved()) {
        this->annealer->solve(puzzle);
    }
}

void MarkovAnnealingGenerator::alterSolution() {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::alterSolution()" << std::endl;
    #endif
    // TODO
}

Puzzle MarkovAnnealingGenerator::generatePuzzle() {
    #ifdef DEBUG_GENERATE_CPP
     std::cout << "MarkovAnnealingGenerator::generatePuzzle()" << std::endl;
    #endif
    unsigned sizeSquared = this->size * this->size;
    unsigned char solution[sizeSquared];
    for (unsigned char *val = this->state, *sol = solution, *solMax = sol + sizeSquared; sol < solMax; val++, sol++)
        *sol = *val;
    while (hasUniqueSolution(Puzzle(this->size, this->state))) {
        unsigned cell = std::rand() % sizeSquared;
        this->state[cell] = 0;
    }
    return Puzzle(this->size, this->state, solution);
}