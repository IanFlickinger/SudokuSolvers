#include "data.h"
#include "puzzle.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <cmath>

#define DEBUG_ENABLED false
#define DEBUG_ENABELD_VERBOSE false
#include "debugging.h"

Puzzle PuzzleLoader::load(unsigned seed) const {
    DEBUG_FUNC_HEADER("PuzzleLoader::loadNew(%d)", seed)
    // select random puzzle
    std::srand(seed);
    unsigned puzzleNumber;
    puzzleNumber = std::rand() * this->randMultiplier;
    puzzleNumber += std::rand() % this->randMultiplier;
    puzzleNumber %= this->datasetSize;
    DEBUG_OUTPUT("Random puzzle number selected: %d", puzzleNumber)
    

    // READ IN PUZZLE
    std::ifstream dataset;
    dataset.open(this->file);
    if (dataset.fail() || dataset.bad() || dataset.eof()) {
        DEBUG_OUTPUT("Error opening dataset file... returning empty puzzle")
        return Puzzle();
    }
    
    // seek puzzleNumber line
    dataset.seekg(SUDOKU_DATASET_HEADER_LINESIZE + this->lineSize * puzzleNumber);

    // read in puzzle line
    std::string puzzleLine;
    dataset >> puzzleLine;
    DEBUG_OUTPUT("Puzzle line read from file: %s", puzzleLine)
    
    // close file
    dataset.close();

    unsigned sizeSquared = puzzleLine.find_first_of(',');

    // DO NOT CONTINUE IF PUZZLE SIZE IS WRONG
    if (sizeSquared != this->puzzleSizeSquared) {
        DEBUG_OUTPUT("Puzzle size mismatch! Returning empty puzzle")
        return Puzzle(this->puzzleSize);
    }

    unsigned char values[sizeSquared];
    unsigned char * solution = new unsigned char[sizeSquared];

    // TODO: define format for sudokus of size > 9... when single chars won't suffice anymore
    //  - Initial thought: leading zeros... but then interpreting size isn't as straightforward
    DEBUG_STATEMENT(std::string valuesBuffer; std::string solutionBuffer)
    for (int i = 0, j = sizeSquared+1; i < sizeSquared; i++, j++) {
        values[i] = puzzleLine.at(i) - '0'; 
        solution[i] = puzzleLine.at(j) - '0';
        DEBUG_STATEMENT(valuesBuffer += std::to_string(values[i]))
        DEBUG_STATEMENT(solutionBuffer += std::to_string(solution[i]))
    }
    
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::cout << "Initial Values: " << valuesBuffer << '\n';
     std::cout << "Solution: " << solutionBuffer << '\n';
     std::cout << std::endl;
    #endif
    DEBUG_OUTPUT("Initial Values: %s", valuesBuffer)
    DEBUG_OUTPUT("Solution: %s", solutionBuffer)

    DEBUG_FUNC_END()
    return Puzzle(this->puzzleSize, values, solution, false);
}

PuzzleDumper::PuzzleDumper(std::string filepath, unsigned char puzzleSize) 
    : filepath(filepath), puzzleSize(puzzleSize) 
{ 
    this->digits = std::ceil(std::log10(puzzleSize));
    startFile(); 
};

void PuzzleDumper::startFile() {
    std::ofstream file(this->filepath, std::ios_base::out);
    file << "Puzzle,Solution" << std::endl;
    file.close();
}

void PuzzleDumper::dump(const Puzzle *puzzles, unsigned num) {
    for (const Puzzle *p = puzzles, *pMax = p + num; p < pMax; p++) this->dump(*p);
}

void PuzzleDumper::dump(const Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("PuzzleDumper::dump(Puzzle&)")
    std::string values, solution;
    for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++) {
        values += std::to_string(puzzle.getValue(cell));
        solution += std::to_string(puzzle.getSolutionAt(cell));
        DEBUG_OUTPUT("Cell %d: value=%d; solution=%d", cell, puzzle.getValue(cell), puzzle.getSolutionAt(cell))
    }
    DEBUG_OUTPUT("Writing to file: %s, %s", values, solution)
    std::ofstream file(this->filepath, std::ios_base::app);
    file << values << ',' << solution << std::endl;
    file.close();
    DEBUG_FUNC_END()
}