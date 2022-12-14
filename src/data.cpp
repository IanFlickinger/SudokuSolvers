#include "data.h"
#include "puzzle.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <cmath>

// #define DEBUG_ENABLED
// #define DEBUG_ENABLED_VERBOSE 
#include "debugging.h"

PuzzleLoader::PuzzleLoader(std::string filepath, unsigned long datasetSize, unsigned char puzzleSize, unsigned seed) : 
    file(filepath), datasetSize(datasetSize), puzzleSize(puzzleSize), puzzleSizeSquared(puzzleSize*puzzleSize), 
    seed(seed), batchSize(0)
{
    DEBUG_FUNC_HEADER("PuzzleLoader::PuzzleLoader(\"%s\", %d, %d, %d)", filepath.c_str(), datasetSize, puzzleSize, seed)
    // TODO: use seed to instantiate a RNG member
    unsigned randMaxLowerBound = RAND_MAX_LOWER_BOUND_FACTOR * datasetSize;
    randMultiplier = randMaxLowerBound > RAND_MAX ? RAND_MAX / randMaxLowerBound : 1;
    puzzleCursor = 0;

    // verify file is csv
    if (filepath.substr(filepath.find_last_of('.')) != ".csv") {
        DEBUG_OUTPUT("ERROR: Given file extension %s is not csv", filepath.substr(filepath.find_last_of('.')).c_str())
        DEBUG_FUNC_END()
        return;
    }

    // scan file
    DEBUG_OUTPUT("Scanning dataset file")
    std::ifstream dataset;
    dataset.open(filepath);
    if (dataset.fail() || dataset.bad() || dataset.eof()) {
        DEBUG_OUTPUT("Error opening dataset file... could not instantiate PuzzleLoader properly")
        DEBUG_FUNC_END()
        return;
    }

    // read in header line size
    std::string header;
    dataset >> header;
    this->headerSize = header.length();
    DEBUG_OUTPUT("Header line has length %d: %s", this->headerSize, header.c_str())

    // confirm puzzle line size
    std::string puzzle;
    dataset >> puzzle;
    this->lineSize = puzzle.length() + 1; // include newline character
    DEBUG_OUTPUT("Puzzle line has length %d: %s", this->lineSize, puzzle.c_str())
    size_t cpos = puzzle.find(',');
    if (cpos == std::string::npos) {
        // no comma in line => line specifies puzzle wo/solution
        if (this->lineSize != this->puzzleSizeSquared + 1) {
            DEBUG_OUTPUT("ERROR: Mismatch between expected puzzle size %d and given puzzle size %d", this->puzzleSizeSquared, this->lineSize)
            DEBUG_FUNC_END()
            return;
        }
    }
    else if (puzzle.find(',', cpos+1) == std::string::npos) {
        // only one comma in line => line specifies puzzle w/solution
        if (this->lineSize != ((this->puzzleSizeSquared + 1) << 1)) {
            DEBUG_OUTPUT("ERROR: Mismatch between expected line size %d and given line size %d", (this->puzzleSizeSquared + 1) << 1, this->lineSize)
            DEBUG_FUNC_END()
            return;
        }
    }
    else {
        // more than one comma in line => violates api-defined format of file
        DEBUG_OUTPUT("ERROR: Too many columns in csv file")
        DEBUG_FUNC_END()
        return;
    }

    // close file
    dataset.close();
    DEBUG_FUNC_END()
};

Puzzle PuzzleLoader::load(unsigned seed) {
    DEBUG_FUNC_HEADER("PuzzleLoader::loadNew(%d)", seed)
    // select random puzzle
    unsigned puzzleNumber = this->puzzleCursor++;

    if (seed && this->seed) {
        std::srand(seed);
        puzzleNumber = std::rand() * this->randMultiplier;
        puzzleNumber += std::rand() % this->randMultiplier;
        puzzleNumber %= this->datasetSize;
    }
    DEBUG_OUTPUT("Puzzle number selected: %d", puzzleNumber)

    // READ IN PUZZLE
    std::ifstream dataset;
    dataset.open(this->file);
    if (dataset.fail() || dataset.bad() || dataset.eof()) {
        DEBUG_OUTPUT("Error opening dataset file... returning empty puzzle")
        DEBUG_FUNC_END()
        return Puzzle();
    }
    
    // seek puzzleNumber line
    dataset.seekg(this->headerSize + this->lineSize * puzzleNumber);

    // read in puzzle line
    std::string puzzleLine;
    dataset >> puzzleLine;
    DEBUG_OUTPUT("Puzzle line read from file: %s", puzzleLine.c_str())
    
    // close file
    dataset.close();

    unsigned sizeSquared = puzzleLine.find_first_of(',');

    // DO NOT CONTINUE IF PUZZLE SIZE IS WRONG
    if (sizeSquared != this->puzzleSizeSquared) {
        DEBUG_OUTPUT("Puzzle size mismatch! Returning empty puzzle")
        DEBUG_FUNC_END()
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
    
    DEBUG_OUTPUT("Initial Values: %s", valuesBuffer.c_str())
    DEBUG_OUTPUT("Solution: %s", solutionBuffer.c_str())

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