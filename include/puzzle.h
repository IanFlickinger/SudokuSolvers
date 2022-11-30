#ifndef SUDOKU_PUZZLE_H
#define SUDOKU_PUZZLE_H

#include <string>

#define COORDS_TO_CELL(row, col, size) row * size + col
#define CELL_TO_COORDS(cell, size) cell / size, cell % size

unsigned perfectSqrt(unsigned);
bool isSudokuSolution(unsigned char, unsigned char*);
bool sudokuHasConflict(unsigned char, unsigned char*);

class Puzzle {
	private:
		unsigned char sizeSqrt; // standard sudoku will be 3
		unsigned char size; // standard sudoku will be 9
		unsigned sizeSquared; // standard sudoku will be 81
		unsigned char * values; // standard sudoku will be flattened 9x9 multidimensional array 
		unsigned char * solution; // standard sudoku will be flattened 9x9 multidimensional array 
		bool * concrete; // 1:1 correspondence with values - whether the value is set 
		
		// For graph representation
		unsigned **neighbors;
		unsigned ***neighborhoods;

		void initializeSize(unsigned char);

	public:
		// Constructors
		Puzzle(); // Default constructor
		Puzzle(unsigned char size); // Size-only constructor
		Puzzle(unsigned char size, unsigned char* values); // Single-dimension array constructor
		Puzzle(unsigned char size, unsigned char* values, unsigned char* solution, bool copySolution = true) // Single-dimension array constructor w/solution
			: Puzzle(size, values) { setSolution(solution, copySolution); }
		Puzzle(unsigned char size, unsigned char** values); // Multi-dimension array constructor
		Puzzle(unsigned char size, unsigned char** values, unsigned char** solution); // Multi-dimension array constructor
		Puzzle(const Puzzle&); // Copy constructor

		// Destructor
		~Puzzle(); 

		// Assignment
		void swap(Puzzle &other);
		Puzzle& operator=(const Puzzle &other) {Puzzle temp(other); swap(temp); return *this; }

		// Accessors
		bool isConcrete(unsigned cell) const {return this->concrete[cell];}
		bool isConcrete(unsigned char row, unsigned char col) const
			{return concrete[COORDS_TO_CELL(row, col, size)];}
		unsigned char getValue(unsigned char row, unsigned char col) const
			{return values[COORDS_TO_CELL(row, col, size)];}
		unsigned char getValue(unsigned cell) const
			{return values[cell];}
		unsigned char getSize() const {return size;}
		unsigned char getSizeSqrt() const {return sizeSqrt;}
		unsigned char getSizeSquared() const {return sizeSquared;}
		// unsigned getNeighborhoodSize() const { return calculateNeighborhoodSize(size); }
		unsigned computeNeighborhoodSize() const {return 3 * (size - 1) - 2 * (sizeSqrt - 1); }

		// Mutators
		bool setValue(unsigned char row, unsigned char col, unsigned char val);
		bool setValue(unsigned cell, unsigned char val);
		bool setSolution(unsigned char* solution, bool copy = true);
		bool setSolution(unsigned char** solution);
		void reset();

		// Graph representation
		const unsigned * neighborsOf(unsigned cell) 
			{ return const_cast<const unsigned *>(neighbors[cell]); }
		const unsigned * neighborsOf(unsigned char row, unsigned char col)
			{ return const_cast<const unsigned *>(neighborsOf(COORDS_TO_CELL(row, col, size))); }
		const unsigned ** neighborList() { return const_cast<const unsigned **>(neighbors); }
		const unsigned *** neighborhoodList() { return const_cast<const unsigned ***>(neighborhoods); }

		// Boolean Queries
		bool isSolved() const;
		bool matchesSolution() const;
		bool hasConflict() const {return sudokuHasConflict(size, values);}
		bool hasConflictInRow(unsigned char row) const;
		bool hasConflictInCol(unsigned char col) const;
		bool hasConflictInBox(unsigned char majorRow, unsigned char majorCol) const;
		bool hasConflictAt(unsigned cell) const;
		bool hasConflictAt(unsigned char row, unsigned char col) const
			{ return hasConflictAt(COORDS_TO_CELL(row, col, size)); }
		unsigned numConflicts() const;
		unsigned numConflictsInRow(unsigned char row) const;
		unsigned numConflictsInCol(unsigned char col) const;
		unsigned numConflictsInBox(unsigned char majorRow, unsigned char majorCol) const;
		unsigned numConflictsAt(unsigned cell) const;
		unsigned numConflictsAt(unsigned char row, unsigned char col) const
			{ return numConflictsAt(COORDS_TO_CELL(row, col, size)); }

		// Friend functions
		std::string to_string();
};

#endif