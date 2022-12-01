#ifndef SUDOKU_DATA_H
#define SUDOKU_DATA_H

#include <string>
#include <fstream>
#include "puzzle.h"

#define RAND_MAX_LOWER_BOUND_FACTOR 10 // 10*DATASET_SIZE to ensure biggest uniform pmf discrepancy is 11:10
#define SUDOKU_DATASET_HEADER_LINESIZE 15 // Puzzle,Solution

class PuzzleLoader {
    private:
        const std::string file;
        const unsigned long datasetSize;
        const unsigned char puzzleSize;
        const unsigned puzzleSizeSquared;
        const unsigned lineSize;
        unsigned randMultiplier;
        unsigned seed;
        unsigned batchSize;
    public:
        PuzzleLoader(std::string filepath, unsigned long datasetSize, unsigned char puzzleSize) : 
            PuzzleLoader(filepath, datasetSize, puzzleSize, time(NULL)) {};
        PuzzleLoader(std::string filepath, unsigned long datasetSize, unsigned char puzzleSize, unsigned seed) : // TODO: use seed to instantiate a RNG member
            file(filepath), datasetSize(datasetSize), puzzleSize(puzzleSize), puzzleSizeSquared(puzzleSize*puzzleSize), 
            lineSize((puzzleSizeSquared + 1) << 1), // 2*puzzleSizeSquared numbers per row + 2 extra characters (',' and '\n')
            seed(seed), batchSize(0) 
        {
            unsigned randMaxLowerBound = RAND_MAX_LOWER_BOUND_FACTOR * datasetSize;
            randMultiplier = randMaxLowerBound > RAND_MAX ? RAND_MAX / randMaxLowerBound : 1;
        };

        // void batch(unsigned batchSize) { this->batchSize = batchSize; }

        // Puzzle * next();

        Puzzle load(unsigned seed) const;
        inline Puzzle load() const { return load(time(NULL)); }
};

class PuzzleDumper {
    private:
        const std::string filepath;
        const unsigned char puzzleSize;
        unsigned char digits;
        std::ofstream file;

        void startFile();
    public:
        PuzzleDumper(std::string filepath, unsigned char puzzleSize);

        void dump(const Puzzle &puzzle);
        void dump(const Puzzle *puzzle, unsigned num);
};

#endif