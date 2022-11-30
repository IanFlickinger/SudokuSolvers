#include "puzzle.h"
#include <fstream>
#include <cmath>
#include <string>
#ifdef DEBUG_PUZZLE_CPP
 #include <iostream>
 #include <iomanip>
 using namespace std;
#endif

template <typename Key, typename Val> struct node_t {
	Key key;
	Val val;
	node_t *next = nullptr;

	node_t() = default;
	inline node_t(Key key, Val val) {this->key = key; this->val = val;}
};

template<typename Key, typename Val> struct cache_t {
	node_t<Key, Val> *listHead = nullptr;
	
	Val get(Key key, Val defaultValue) {
		node_t<Key, Val> *cursor = listHead;
		while (cursor != nullptr) {
			if (key == cursor->key) return cursor->val;
			cursor = cursor->next;
		}
		return defaultValue;
	}

	bool add(Key key, Val val) {
		if (listHead == nullptr) {
			listHead = new node_t<Key, Val>(key, val);
			return true;
		}
		node_t<Key, Val> *cursor = listHead;
		while (true) {
			if (cursor->key == key) return false;
			if (cursor->next == nullptr) break;
			cursor = cursor->next;
		}
		cursor->next = new node_t<Key, Val>(key, val);
		return true;
	}

	~cache_t() {
		if (listHead == nullptr) return;
		node_t<Key, Val> * cursor = listHead, *last = cursor;
		while (cursor != nullptr) {
			cursor = cursor->next;
			delete last;
			last = cursor;
		}
	}
};


unsigned perfectSqrt(unsigned square) {
	unsigned val = sqrt(square);
	// Only return value if square is a perfect square
	return val*val == square ? val : 0;
}

bool isSudokuSolution(unsigned char size, unsigned char* solution) {
	// This function is not as efficient as the class built-in,
	// but covers more cases.

	// Check for invalid values (including zeros)
	for (unsigned cell = 0; cell < size * size; cell++)
		if (solution[cell] == 0 || solution[cell] > size) return false;

	// Check for conflicts
	return !sudokuHasConflict(size, solution);
}

bool sudokuHasConflict(unsigned char size, unsigned char* solution) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "sudokuHasConflict(" << static_cast<int>(size) << ", unsigned char*)" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	unsigned sizeSquared = size * size;
	unsigned char sizeSqrt = perfectSqrt(size);
	// Check rows for duplicates
	for (unsigned char row = 0; row < size; row++) {
		unsigned rowOffset = row * size;
		unsigned char vals[size+1] = {0};
		for (unsigned cell = rowOffset; cell < rowOffset + size; cell++)
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0) 
				#ifdef DEBUG_PUZZLE_CPP_VERBOSE
				{
					cout << "Row conflict found: multiple " << to_string(solution[cell]) << " instances in row " 
						 << to_string(row) << '\n' << endl;
					return true;
				}
				#else
					return true;
				#endif
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << "No row conflicts found\n";
	#endif

	// Check cols for duplicates
	for (unsigned char col = 0; col < size; col++) {
		unsigned char vals[size+1] = {0};
		for (unsigned cell = col; cell < sizeSquared; cell += size)
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0)
				#ifdef DEBUG_PUZZLE_CPP_VERBOSE
				{
					cout << "Column conflict found: multiple " << to_string(solution[cell]) << " instances in column " 
						 << to_string(col) << '\n' << endl;
					return true;
				}
				#else
					return true;
				#endif
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << "No column conflicts found\n";
	#endif

	// Check boxes for duplicates
	for (unsigned char majorRow = 0, majorCol = 0; majorRow < sizeSqrt; 
		majorCol = (majorCol + 1) % sizeSqrt, majorRow += (majorCol == 0)
	) {
		unsigned boxOffset = majorRow * size * sizeSqrt + majorCol * sizeSqrt;
		unsigned char vals[size+1] = {0};
		for (unsigned cell = boxOffset, minorCol = 0; cell < boxOffset + sizeSqrt * size; 
			minorCol = (minorCol + 1) % sizeSqrt, cell = cell + 1 + (size - sizeSqrt) * (minorCol == 0)
		) 
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0)
				#ifdef DEBUG_PUZZLE_CPP_VERBOSE
				{
					cout << "Box conflict found: multiple " << to_string(solution[cell]) << " instances in box " 
						 << to_string(majorRow) << to_string(majorCol) << '\n' << endl;
					return true;
				}
				#else
					return true;
				#endif
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << "No box conflicts found\n" << endl;
	#endif
	return false;
}

void Puzzle::initializeSize(unsigned char size) {
	this->size = size;
	this->sizeSqrt = perfectSqrt(size);
	this->sizeSquared = size * size;
	this->values = new unsigned char[this->sizeSquared];
	this->concrete = new bool[this->sizeSquared];
	this->solution = nullptr;

	// BUILD NEIGHBORHOODS
	// TODO: several optimizations are possible
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		cout << "Building neighborhoods" << endl;
	#endif
	unsigned neighborSize = this->computeNeighborhoodSize();
	unsigned **neighbors = new unsigned*[this->sizeSquared];
	for (unsigned **i = neighbors, **iMax = i + this->sizeSquared; i < iMax; i++) *i = new unsigned[neighborSize];

	unsigned neighborhoodSize = this->size - 1;
	unsigned ***neighborhoods = new unsigned**[this->sizeSquared];
	for (unsigned ***i = neighborhoods, ***iMax = i + this->sizeSquared; i < iMax; i++) {
		*i = new unsigned*[3]; 
		for (unsigned **j = *i, **jMax = j + 3; j < jMax; j++) 
			*j = new unsigned[neighborhoodSize];
	}

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		cout << "MemoryAllocated" << endl;
	#endif

	unsigned cell = 0, rowOffset = 0, rowCeiling = this->size;
	unsigned boxOffset = 0, boxCeiling = (this->sizeSqrt - 1) * this->size + this->sizeSqrt;
	for (unsigned char row = 0, col = 0, majorRow = 0, majorCol = 0, minorRow = 0, minorCol = 0; 
		cell < this->sizeSquared; cell++
	) {
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
			cout << "Cell " << cell << ":" << endl;
		#endif
		unsigned neighborOffset = 0;

		// build row neighborhood
		for (unsigned otherCell = rowOffset, i = 0; otherCell < rowCeiling; otherCell++) {
			if (otherCell != cell) {
				neighbors[cell][neighborOffset++] = otherCell;
				neighborhoods[cell][0][i++] = otherCell;
			}
		}
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
			cout << "\tRow neighborhood complete" << endl;
		#endif

		// build col neighborhood
		for (unsigned otherCell = col, i = 0; otherCell < this->sizeSquared; otherCell += this->size)
			if (otherCell != cell) {
				neighbors[cell][neighborOffset++] = otherCell;
				neighborhoods[cell][1][i++] = otherCell;
			}
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
			cout << "\tColumn neighborhood complete" << endl;
		#endif
		
		// build box neighborhood
		unsigned char otherMinorRow = 0, otherMinorCol = 0;
		for (unsigned otherCell = boxOffset, i = 0; 
				otherMinorRow < this->sizeSqrt; 
				otherMinorCol++, otherCell++
		) {
			if (otherMinorCol == this->sizeSqrt) {
				otherMinorCol = 0; otherMinorRow++;
				otherCell += this->size - this->sizeSqrt;
			}
			if (otherCell != cell) {
				if (otherMinorCol != minorCol && otherMinorRow != minorRow) 
					neighbors[cell][neighborOffset++] = otherCell;
				neighborhoods[cell][2][i++] = otherCell;
			}
		}
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
			cout << "\tBox neighborhood complete" << endl;
		#endif

		// update loop variants
		minorCol++; col++;
		if (minorCol == this->sizeSqrt) {
			minorCol = 0; majorCol++;
			boxOffset += this->sizeSqrt;
			boxCeiling += this->sizeSqrt;
			if (col == this->size) {
				col = majorCol = 0;
				minorRow++; row++;
				rowOffset += this->size;
				rowCeiling += this->size;
				boxOffset -= this->size;
				boxCeiling -= this->size;
				if (minorRow == this->sizeSqrt) {
					minorRow = 0; majorRow++;
					unsigned majorSize = this->sizeSqrt * this->size;
					boxOffset += majorSize;
					boxCeiling += majorSize;
				}
			}
		}
	}
	this->neighbors = neighbors;
	this->neighborhoods = neighborhoods;
}

Puzzle::Puzzle() {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::Puzzle()" << endl;
	#endif
	// setting size to 0 prevents memory errors in array access functions
	this->sizeSqrt = 0;
	this->size = 0;
	this->sizeSquared = 0;
	this->values = nullptr;
	this->concrete = nullptr;
	this->solution = nullptr;
	this->neighbors = nullptr;
	this->neighborhoods = nullptr;
}

Puzzle::Puzzle(unsigned char size) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::Puzzle(" << static_cast<int>(size) << ", unsigned char*)" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	this->initializeSize(size);
	for (int cell = 0; cell < this->sizeSquared; cell++) {
		this->values[cell] = 0;
		this->concrete[cell] = false;
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		 unsigned r = cell / size;
		 unsigned c = cell % size;
		 cout << "Assigning " << 0
		      << " to row " << r << " and column " << c << endl;
		 cout << "\tvalues[" << r << "][" << c << "] == " << static_cast<int>(this->values[cell]) << endl;
		 cout << "\tconcrete[" << r << "][" << c << "] == " << this->concrete[cell] << endl;
		#endif
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << endl;
	#endif
}

Puzzle::Puzzle(unsigned char size, unsigned char *values) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::Puzzle(" << static_cast<int>(size) << ", unsigned char*)" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	this->initializeSize(size);

	for (int cell = 0; cell < this->sizeSquared; cell++) {
		unsigned char value = values[cell];
		if (value > size) value = 0;
		this->values[cell] = value;
		this->concrete[cell] = (value != 0);
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		unsigned r = cell / size;
		unsigned c = cell % size;
		cout << "Assigning " << static_cast<unsigned>(value) 
			<< " to row " << r << " and column " << c << endl;
		cout << "\tvalues[" << r << "][" << c << "] == " << static_cast<int>(this->values[cell]) << endl;
		cout << "\tconcrete[" << r << "][" << c << "] == " << this->concrete[cell] << endl;
		#endif
	}

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << endl;
	#endif
}

Puzzle::Puzzle(unsigned char size, unsigned char **values) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::Puzzle(unsigned char[][])" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	this->initializeSize(size);

	for (int cell = 0, r = 0, c = 0; r < size; cell++, c = (c+1) % size, r = r+(c==0)) {
		unsigned char value = values[r][c];
		value = value > size ? 0 : value;
		this->values[cell] = value;
		this->concrete[cell] = (value != 0);
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		 cout << "Assigning " << static_cast<unsigned>(value) 
		      << " to row " << r << " and column " << c << endl;
		 cout << "\tvalues[" << r << "][" << c << "] == " << static_cast<int>(this->values[cell]) << endl;
		 cout << "\tconcrete[" << r << "][" << c << "] == " << this->concrete[cell] << endl;
		#endif

	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << endl;
	#endif
}

Puzzle::Puzzle(const Puzzle &other) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::Puzzle(const Puzzle &other)" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	this->initializeSize(other.size);
	// TODO: handle neighbors for efficiency?
	// TODO: handle solution
	for (int cell = 0; cell < this->sizeSquared; cell++) {
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		 unsigned r = cell / this->size;
		 unsigned c = cell % this->size;
		 cout << "Assigning " << static_cast<unsigned>(other.values[cell]) 
		      << " to row " << r << " and column " << c << endl;
		#endif
		this->values[cell] = other.values[cell];
		this->concrete[cell] = other.concrete[cell];
		#ifdef DEBUG_PUZZLE_CPP_VERBOSE
		 cout << "\tvalues[" << r << "][" << c << "] == " << static_cast<int>(this->values[cell]) << endl;
		 cout << "\tconcrete[" << r << "][" << c << "] == " << this->concrete[cell] << endl;
		#endif
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << endl;
	#endif
}

bool Puzzle::setValue(unsigned char row, unsigned char col, unsigned char val) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::setValue(" << static_cast<int>(row) << ", " << static_cast<int>(col) 
	      << ", " << static_cast<int>(val) << ")" << endl;
	#endif
	unsigned cell = COORDS_TO_CELL(row, col, this->size);
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	 if (concrete[cell]) cout << "Cell is concrete! Cannot reassign!\n" << endl;
	 if (val > this->size) cout << "ERROR: Value " << static_cast<int>(val) << " is out of range!\n" << endl;
	#endif
	if (concrete[cell] || val > this->size) 
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	{
		cout << endl;
		return false;
	}
	#else
		return false;
	#endif
	this->values[cell] = val;
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << "New value at row " << static_cast<int>(row) << " and column " << static_cast<int>(col) 
	      << ": " << static_cast<int>(this->values[cell]) << "\n\n";
	#endif
	return true;
}

bool Puzzle::setValue(unsigned cell, unsigned char val) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::setValue(" << cell << ", " << static_cast<int>(val) << ")" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	 if (concrete[cell]) cout << "Cell is concrete! Cannot reassign!" << endl;
	 if (val > this->size) cout << "ERROR: Value " << static_cast<int>(val) << " is out of range!" << endl;
	 if (!concrete[cell] && val <= this->size) cout << "Reassigning value..." << endl;
	#endif
	if (concrete[cell] || val > this->size) 
		return false;
	this->values[cell] = val;
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << "New value at row " << cell / size << " and column " << cell % size
	      << ": " << static_cast<int>(this->values[cell]) << "\n" << endl;
	#endif
	return true;
}

bool Puzzle::setSolution(unsigned char *solution, bool copy) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::setSolution(unsigned char*, copy=" << std::to_string(copy) << ")" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	// No solution overriding allowed at this point: should be unnecessary
	// TODO: Macro which fail conditions with explanations?
	// TODO: Reject if solution does not match values?
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	if (this->solution != nullptr) {
		cout << "ERROR: Solution override request denied\n" << endl;
		return false;
	}
	if (!isSudokuSolution(this->size, solution)) {
		cout << "ERROR: Invalid solution provided\n" << endl;
		return false;
	}
	#else
	if (this->solution != nullptr or !isSudokuSolution(this->size, solution)) 
		return false;
	#endif
	if (copy) {
		this->solution = new unsigned char[this->sizeSquared];
		for (unsigned cell = 0; cell < this->sizeSquared; cell++) {
			this->solution[cell] = solution[cell];
		}
	}
	else this->solution = solution;
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout  << endl;
	#endif
	return true;
}

bool Puzzle::setSolution(unsigned char **solution) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::setSolution(unsigned char**)" << endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	// No solution overriding allowed at this point: should be unnecessary
	if (this->solution != nullptr) return false;
	this->solution = new unsigned char[this->sizeSquared];
	for (unsigned cell = 0, row = 0, col = 0; cell < this->sizeSquared; 
			col = (col + 1) % this->size, row += (col == 0), cell++
		) this->solution[cell] = solution[row][col];
	if (!isSudokuSolution(this->size, this->solution)) {
		delete[] this->solution;
		this->solution = nullptr;
	}
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 cout  << endl;
	#endif
	return true;
}

void Puzzle::reset() {
	for (unsigned cell = 0; cell < this->sizeSquared; cell++) this->values[cell] *= this->concrete[cell];
}

bool Puzzle::isSolved() const {
	if (solution == nullptr) return isSudokuSolution(size, values);
	for (unsigned cell = 0; cell < sizeSquared; cell++) 
		if (values[cell] != solution[cell]) return false;
	return true;
}

bool Puzzle::matchesSolution() const {
	// Returns true if all nonempty cells match provided solution
	// Currently returns false if no solution is provided
	if (solution == nullptr) return false;
	for (unsigned cell = 0; cell < sizeSquared; cell++) 
		if (values[cell] != solution[cell] && values[cell] != 0) return false;
	return true;
}

bool Puzzle::hasConflictInRow(unsigned char row) const {
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::hasConflictInRow(" << std::to_string(row) << ")" << std::endl;
	#endif
	unsigned rowOffset = row * size;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = rowOffset; cell < rowOffset + size; cell++)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) return true;
	return false;
}

bool Puzzle::hasConflictInCol(unsigned char col) const {
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::hasConflictInCol(" << std::to_string(col) << ")" << std::endl;
	#endif
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = col; cell < sizeSquared; cell += size)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) return true;
	return false;
}

bool Puzzle::hasConflictInBox(unsigned char majorRow, unsigned char majorCol) const {
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::hasConflictInBox(" << std::to_string(majorRow) << ", " << std::to_string(majorCol) << ")" << std::endl;
	#endif
	unsigned char vals[size+1] = {0};
	unsigned majorSize = size * sizeSqrt;
	unsigned boxOffset = majorRow * majorSize + majorCol * sizeSqrt;
	for (unsigned char cell = boxOffset, minorCol = 0; cell < boxOffset + majorSize; 
		minorCol = (minorCol + 1) % sizeSqrt, cell = cell + 1 + (size - sizeSqrt) * (minorCol == 0)
	)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) return true;
	return false;
}

bool Puzzle::hasConflictAt(unsigned cell) const {
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::hasConflictAt(" << cell << ")" << std::endl;
	#endif
	// get cell value
	unsigned value = this->values[cell];

	// no conflict if cell is empty
	if (value == 0) return false;

	// iterate over list of neighbors
	unsigned *neighbor = this->neighbors[cell];
	unsigned *neighborCeiling = neighbor + this->computeNeighborhoodSize();
	for ( ; neighbor < neighborCeiling; neighbor++) 
		// conflict found if any neighbor shares the same value
		if (this->values[*neighbor] == value) return true;
	
	// no conflict found
	return false;
}

unsigned Puzzle::numConflictsInRow(unsigned char row) const {
	unsigned conflicts = 0;
	unsigned rowOffset = row * size;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = rowOffset; cell < rowOffset + size; cell++)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflictsInCol(unsigned char col) const {
	unsigned conflicts = 0;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = col; cell < sizeSquared; cell += size)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflictsInBox(unsigned char majorRow, unsigned char majorCol) const {
	unsigned conflicts = 0;
	unsigned char vals[size+1] = {0};
	unsigned majorSize = size * sizeSqrt;
	unsigned boxOffset = majorRow * majorSize + majorCol * sizeSqrt;
	for (unsigned char cell = boxOffset, minorCol = 0; cell < boxOffset + majorSize; 
		minorCol = (minorCol + 1) % sizeSqrt, cell = cell + 1 + (size - sizeSqrt) * (minorCol == 0)
	)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflictsAt(unsigned cell) const {
	unsigned value = this->values[cell], conflicts = 0;
	unsigned *neighbor = this->neighbors[0];
	unsigned *neighborCeiling = neighbor + this->computeNeighborhoodSize();
	for (unsigned * neighbor = this->neighbors[0]; neighbor < neighborCeiling; neighbor++)
		if (*neighbor == value) conflicts++;
	return false;
}

unsigned Puzzle::numConflicts() const {
	unsigned conflicts = 0;
	for (unsigned char row = 0; row < size; row++) {
		unsigned rowOffset = row * size;
		unsigned char vals[size+1] = {0};
		for (unsigned char cell = rowOffset; cell < rowOffset + size; cell++)
			if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	}
	for (unsigned char col = 0; col < size; col++) {
		unsigned char vals[size+1] = {0};
		for (unsigned char cell = col; cell < sizeSquared; cell += size)
			if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	}
	for (unsigned char majorRow = 0, majorCol = 0; majorRow < sizeSqrt; 
		majorCol = (majorCol + 1) % sizeSqrt, majorRow += (majorCol == 0)
	) {
		unsigned char vals[size+1] = {0};
		unsigned majorSize = size * sizeSqrt;
		unsigned boxOffset = majorRow * majorSize + majorCol * sizeSqrt;
		for (unsigned char cell = boxOffset, minorCol = 0; cell < boxOffset + majorSize; 
			minorCol = (minorCol + 1) % sizeSqrt, cell = cell + 1 + (size - sizeSqrt) * (minorCol == 0)
		)
			if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	}
	return conflicts;
}

void Puzzle::swap(Puzzle &other) {
	#ifdef DEBUG_PUZZLE_CPP
	 cout << "Puzzle::swap(Puzzle&)" << endl;
	#endif
	std::swap(size, other.size);
	std::swap(sizeSqrt, other.sizeSqrt);
	std::swap(sizeSquared, other.sizeSquared);
	std::swap(values, other.values);
	std::swap(concrete, other.concrete);
	std::swap(solution, other.solution);
	std::swap(neighbors, other.neighbors);
	std::swap(neighborhoods, other.neighborhoods);
}

Puzzle::~Puzzle() {
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::~Puzzle()" << std::endl;
	#endif
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 std::cout << setw(55) << setfill('=') << "" << std::endl;
	#endif

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	 if (!this->size) std::cout << "Puzzle not yet initialized... returning early" << std::endl;
	#endif
	if (this->values == nullptr) return;

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	std::cout << "Deleting values and concrete arrays" << std::endl;
	#endif
	delete[] this->values;
	delete[] this->concrete;

	if (this->solution != nullptr)  
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	{
		std::cout << "Deleting solution" << std::endl;
	#endif
		delete[] this->solution;
	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	}
	#endif

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	std::cout << "Deleting Neighbors" << std::endl;
	#endif
	for (unsigned **i = this->neighbors, **iMax = i + this->sizeSquared; i < iMax; i++) delete[] *i;
	delete[] this->neighbors;

	#ifdef DEBUG_PUZZLE_CPP_VERBOSE
	std::cout << "Deleting Neighborhoods" << std::endl;
	#endif
	for (unsigned ***i = this->neighborhoods, ***iMax = i + this->sizeSquared; i < iMax; i++) {
		for (unsigned **j = *i, **jMax = j + 3; j < jMax; j++) {
			delete[] *j;
		}
		delete[] *i;
	}
	delete[] this->neighborhoods;
}

std::string Puzzle::to_string() {
	unsigned char digits = std::log10(this->size);
	return "PUZZLE STRING!!!";
}