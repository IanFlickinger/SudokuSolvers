#include "data.h"
#include "puzzle.h"
#include <string>
#include <fstream>

#ifdef DEBUG_DATA_CPP
 #include <iostream>
 #include <iomanip>
#endif

Puzzle PuzzleLoader::load(unsigned seed) const {
    #ifdef DEBUG_DATA_CPP
     std::cout << "PuzzleLoader::loadNew(" << seed << ")\n";
    #endif
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::cout << std::setw(50) << std::setfill('=') << "\n";
    #endif
    // select random puzzle
    std::srand(seed);
    unsigned puzzleNumber;
    puzzleNumber = std::rand() * this->randMultiplier;
    puzzleNumber += std::rand() % this->randMultiplier;
    puzzleNumber %= this->datasetSize;
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::cout << "Random puzzle number selected: " << puzzleNumber << std::endl;
    #endif
    

    // READ IN PUZZLE
    std::ifstream dataset;
    dataset.open(this->file);
    if (dataset.fail() || dataset.bad() || dataset.eof()) 
    #ifdef DEBUG_DATA_CPP_VERBOSE
    {
        std::cout << "Error opening dataset file... returning empty puzzle\n";
    #endif
        return Puzzle();
    #ifdef DEBUG_DATA_CPP_VERBOSE
    }
    #endif
    
    // seek puzzleNumber line
    dataset.seekg(SUDOKU_DATASET_HEADER_LINESIZE + this->lineSize * puzzleNumber);

    // read in puzzle line
    std::string puzzleLine;
    dataset >> puzzleLine;
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::cout << "Puzzle line read from file: " << "\n\t" << puzzleLine << std::endl;
    #endif
    
    // close file
    dataset.close();

    unsigned sizeSquared = puzzleLine.find_first_of(',');

    // DO NOT CONTINUE IF PUZZLE SIZE IS WRONG
    #ifdef DEBUG_DATA_CPP_VERBOSE
     if (sizeSquared != this->puzzleSize * this->puzzleSize) 
        std::cout << "Puzzle Size Mismatch! Returning empty puzzle" << std::endl;
    #endif
    if (sizeSquared != this->puzzleSizeSquared) return Puzzle(this->puzzleSize);

    unsigned char values[sizeSquared];
    unsigned char * solution = new unsigned char[sizeSquared];

    // TODO: define format for sudokus of size > 9... when single chars won't suffice anymore
    //  - Initial thought: leading zeros... but then interpreting size isn't as straightforward
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::string valuesBuffer, solutionBuffer;
    #endif
    for (int i = 0, j = sizeSquared+1; i < sizeSquared; i++, j++) {
        values[i] = puzzleLine.at(i) - '0'; 
        solution[i] = puzzleLine.at(j) - '0';
        #ifdef DEBUG_DATA_CPP_VERBOSE
         valuesBuffer += std::to_string(values[i]);
         solutionBuffer += std::to_string(solution[i]);
        #endif
    }
    
    #ifdef DEBUG_DATA_CPP_VERBOSE
     std::cout << "Initial Values: " << valuesBuffer << '\n';
     std::cout << "Solution: " << solutionBuffer << '\n';
     std::cout << std::endl;
    #endif
    // return Puzzle(perfectSqrt(sizeSquared), values);
    return Puzzle(this->puzzleSize, values, solution, false);
}