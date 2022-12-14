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
        unsigned lineSize;
        unsigned headerSize;
        unsigned randMultiplier;
        unsigned seed;
        unsigned batchSize;
        unsigned puzzleCursor;

    public:
        PuzzleLoader(std::string filepath, unsigned long datasetSize, unsigned char puzzleSize) : 
            PuzzleLoader(filepath, datasetSize, puzzleSize, time(NULL)) {};
        PuzzleLoader(std::string filepath, unsigned long datasetSize, unsigned char puzzleSize, unsigned seed);

        // void batch(unsigned batchSize) { this->batchSize = batchSize; }

        // Puzzle * next();

        Puzzle load(unsigned seed);
        inline Puzzle load() { return load(0); }
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