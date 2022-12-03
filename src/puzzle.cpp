#include "puzzle.h"
#include <fstream>
#include <cmath>
#include <string>

#define DEBUG_ENABLED false
#define DEBUG_ENABLED_VERBOSE false
#include "debugging.h"

template <typename Key, typename Val> 
struct node_t {
	Key key;
	Val val;
	node_t *next = nullptr;

	node_t() = default;
	inline node_t(Key key, Val val) {this->key = key; this->val = val;}
};

template<typename Key, typename Val> 
struct cache_t {
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
	// This function is not as efficient as the class member function,
	// but covers more cases.

	// Check for invalid values (including zeros)
	for (unsigned cell = 0; cell < size * size; cell++)
		if (solution[cell] == 0 || solution[cell] > size) return false;

	// Check for conflicts
	return !sudokuHasConflict(size, solution);
}

bool sudokuHasConflict(unsigned char size, unsigned char* solution) {
	DEBUG_FUNC_HEADER("sudokuHasConflict(%d, unsigned char*)", size)
	unsigned sizeSquared = size * size;
	unsigned char sizeSqrt = perfectSqrt(size);
	// Check rows for duplicates
	for (unsigned char row = 0; row < size; row++) {
		unsigned rowOffset = row * size;
		unsigned char vals[size+1] = {0};
		for (unsigned cell = rowOffset; cell < rowOffset + size; cell++)
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0) {
					DEBUG_OUTPUT("Row conflict found: multiple %d instances in row %d", solution[cell], row)
					DEBUG_FUNC_END()
					return true;
				}
	}
	DEBUG_OUTPUT("No row conflicts found")

	// Check cols for duplicates
	for (unsigned char col = 0; col < size; col++) {
		unsigned char vals[size+1] = {0};
		for (unsigned cell = col; cell < sizeSquared; cell += size)
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0) {
					DEBUG_OUTPUT("Column conflict found: multiple %d instances in column %d", solution[cell], col)
					DEBUG_FUNC_END()
					return true;
				}
	}
	DEBUG_OUTPUT("No column conflicts found")

	// Check boxes for duplicates
	for (unsigned char majorRow = 0, majorCol = 0; majorRow < sizeSqrt; 
		majorCol = (majorCol + 1) % sizeSqrt, majorRow += (majorCol == 0)
	) {
		unsigned boxOffset = majorRow * size * sizeSqrt + majorCol * sizeSqrt;
		unsigned char vals[size+1] = {0};
		for (unsigned cell = boxOffset, minorCol = 0; cell < boxOffset + sizeSqrt * size; 
			minorCol = (minorCol + 1) % sizeSqrt, cell = cell + 1 + (size - sizeSqrt) * (minorCol == 0)
		) 
			if (vals[solution[cell]]++ > 0 && solution[cell] != 0){
					DEBUG_OUTPUT("Box conflict found: multiple %d instances in box row %d and column %d", solution[cell], majorRow, majorCol)
					DEBUG_FUNC_END()
					return true;
				}
	}
	DEBUG_OUTPUT("No box conflicts found")

	DEBUG_FUNC_END()
	return false;
}

void Puzzle::initializeSize(unsigned char size) {
	DEBUG_FUNC_HEADER("Puzzle::initializeSize(%d)", size)
	this->size = size;
	this->sizeSqrt = perfectSqrt(size);
	this->sizeSquared = size * size;
	this->values = new unsigned char[this->sizeSquared];
	this->concrete = new bool[this->sizeSquared];
	this->solution = nullptr;

	// BUILD NEIGHBORHOODS
	// TODO: several optimizations are possible
	DEBUG_OUTPUT("Building neighborhoods")
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
		std::cout << "MemoryAllocated" << std::endl;
	#endif

	unsigned cell = 0, rowOffset = 0, rowCeiling = this->size;
	unsigned boxOffset = 0, boxCeiling = (this->sizeSqrt - 1) * this->size + this->sizeSqrt;
	for (unsigned char row = 0, col = 0, majorRow = 0, majorCol = 0, minorRow = 0, minorCol = 0; 
		cell < this->sizeSquared; cell++
	) {
		DEBUG_OUTPUT("Cell %d:", cell)
		DEBUG_INDENT()
		unsigned neighborOffset = 0;

		// build row neighborhood
		for (unsigned otherCell = rowOffset, i = 0; otherCell < rowCeiling; otherCell++) {
			if (otherCell != cell) {
				neighbors[cell][neighborOffset++] = otherCell;
				neighborhoods[cell][0][i++] = otherCell;
			}
		}
		DEBUG_OUTPUT("Row neighborhood complete")

		// build col neighborhood
		for (unsigned otherCell = col, i = 0; otherCell < this->sizeSquared; otherCell += this->size)
			if (otherCell != cell) {
				neighbors[cell][neighborOffset++] = otherCell;
				neighborhoods[cell][1][i++] = otherCell;
			}
		DEBUG_OUTPUT("Column neighborhood complete")
		
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
		DEBUG_OUTPUT("Box neighborhood complete")
		DEBUG_OUTDENT()

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

	DEBUG_FUNC_END()
}

Puzzle::Puzzle() {
	DEBUG_OUTPUT("Puzzle::Puzzle()")
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
	DEBUG_FUNC_HEADER("Puzzle::Puzzle(%d)", size)
	this->initializeSize(size);
	for (int cell = 0; cell < this->sizeSquared; cell++) {
		DEBUG_STATEMENT(unsigned r = cell / size)
		DEBUG_STATEMENT(unsigned c = cell % size)
		DEBUG_OUTPUT("Assigning 0 to row %d and column %d", r, c)
		this->values[cell] = 0;
		this->concrete[cell] = false;

		DEBUG_INDENT()
		  DEBUG_OUTPUT("values[%d][%d] == %d", r, c, this->values[cell])
		  DEBUG_OUTPUT("concrete[%d][%d] == %s", r, c, this->concrete[cell] ? "true" : "false")
		DEBUG_OUTDENT()
	}
	DEBUG_FUNC_END()
}

Puzzle::Puzzle(unsigned char size, unsigned char *values) {
	DEBUG_FUNC_HEADER("Puzzle::Puzzle(%d, unsigned char*)", size)
	this->initializeSize(size);

	for (int cell = 0; cell < this->sizeSquared; cell++) {
		unsigned char value = values[cell];
		if (value > size) value = 0;

		DEBUG_STATEMENT(unsigned r = cell / size)
		DEBUG_STATEMENT(unsigned c = cell % size)
		DEBUG_OUTPUT("Assigning %d to row %d and column %d", value, r, c)
		this->values[cell] = value;
		this->concrete[cell] = (value != 0);

		DEBUG_INDENT()
		  DEBUG_OUTPUT("values[%d][%d] == %d", r, c, this->values[cell])
		  DEBUG_OUTPUT("concrete[%d][%d] == %s", r, c, this->concrete[cell] ? "true" : "false")
		DEBUG_OUTDENT()
	}

	DEBUG_FUNC_END()
}

Puzzle::Puzzle(unsigned char size, unsigned char **values) {
	DEBUG_FUNC_HEADER("Puzzle::Puzzle(%d, unsigned char**)", size)
	this->initializeSize(size);

	for (int cell = 0, r = 0, c = 0; r < size; cell++, c = (c+1) % size, r = r+(c==0)) {
		unsigned char value = values[r][c];
		value = value > size ? 0 : value;

		DEBUG_OUTPUT("Assigning %d to row %d and column %d", value, r, c)
		this->values[cell] = value;
		this->concrete[cell] = (value != 0);
		
		DEBUG_INDENT()
		  DEBUG_OUTPUT("values[%d][%d] == %d", r, c, this->values[cell])
		  DEBUG_OUTPUT("concrete[%d][%d] == %s", r, c, this->concrete[cell] ? "true" : "false")
		DEBUG_OUTDENT()
	}

	DEBUG_FUNC_END()
}

Puzzle::Puzzle(const Puzzle &other) {
	DEBUG_FUNC_HEADER("Puzzle::Puzzle(Puzzle&)")
	this->initializeSize(other.size);
	// TODO: handle neighbors for efficiency?
	// TODO: handle solution
	for (int cell = 0; cell < this->sizeSquared; cell++) {
		DEBUG_STATEMENT(unsigned r = cell / size)
		DEBUG_STATEMENT(unsigned c = cell % size)
		DEBUG_OUTPUT("Assigning %d to row %d and column %d", other.values[cell], r, c)
		this->values[cell] = other.values[cell];
		this->concrete[cell] = other.concrete[cell];
		
		DEBUG_INDENT()
		  DEBUG_OUTPUT("values[%d][%d] == %d", r, c, this->values[cell])
		  DEBUG_OUTPUT("concrete[%d][%d] == %s", r, c, this->concrete[cell] ? "true" : "false")
		DEBUG_OUTDENT()
	}

	DEBUG_FUNC_END()
}

bool Puzzle::setValue(unsigned char row, unsigned char col, unsigned char val) {
	DEBUG_FUNC_HEADER("Puzzle::setValue(%d, %d, %d)", row, col, val)
	unsigned cell = COORDS_TO_CELL(row, col, this->size);

	DEBUG_OUTPUT_IF(concrete[cell], "Cell is concrete! Cannot reassign!")
	DEBUG_OUTPUT_IF(val > this->size, "ERROR: Value %d is out of range!", val)
	if (concrete[cell] || val > this->size) {
		DEBUG_FUNC_END()
		return false;
	}
	this->values[cell] = val;

	DEBUG_OUTPUT("New value at row %d and column %d: %d", row, col, this->values[cell])
	DEBUG_FUNC_END()
	return true;
}

bool Puzzle::setValue(unsigned cell, unsigned char val) {
	DEBUG_FUNC_HEADER("Puzzle::setValue(%d, %d", cell, val)
	#ifdef DEBUG_PUZZLE_CPP
	 std::cout << "Puzzle::setValue(" << cell << ", " << static_cast<int>(val) << ")" << std::endl;
	#endif
	DEBUG_OUTPUT_IF(concrete[cell], "Cell is concrete! Cannot reassign!")
	DEBUG_OUTPUT_IF(val > this->size, "ERROR: Value %d is out of range!", val)
	if (concrete[cell] || val > this->size) {
		DEBUG_FUNC_END()
		return false;
	}
	DEBUG_OUTPUT("Reassigning value...")
	this->values[cell] = val;

	DEBUG_OUTPUT("New value at row %d and column %d: %d", cell / size, cell % size, this->values[cell])
	DEBUG_FUNC_END()
	return true;
}

bool Puzzle::setSolution(unsigned char *solution, bool copy) {
	DEBUG_FUNC_HEADER("Puzzle::setSolution(unsigned char*, copy=%s)", copy ? "true": "false")

	// No solution overriding allowed at this point: should be unnecessary
	// TODO: Macro which fail conditions with explanations?
	// TODO: Reject if solution does not match values?
	DEBUG_OUTPUT_IF(this->solution != nullptr, "ERROR: Solution override request denied")
	DEBUG_OUTPUT_IF(!isSudokuSolution(this->size, solution), "ERROR: Invalid solution provided")
	if (this->solution != nullptr || !isSudokuSolution(this->size, solution)) {
		DEBUG_FUNC_END()
		return false;
	}
	if (copy) {
		this->solution = new unsigned char[this->sizeSquared];
		for (unsigned cell = 0; cell < this->sizeSquared; cell++) {
			this->solution[cell] = solution[cell];
		}
	}
	else this->solution = solution;
	
	DEBUG_FUNC_END()
	return true;
}

void Puzzle::reset() {
	DEBUG_OUTPUT("Puzzle::reset()")
	for (unsigned cell = 0; cell < this->sizeSquared; cell++) this->values[cell] *= this->concrete[cell];
}

bool Puzzle::isSolved() const {
	DEBUG_OUTPUT("Puzzle::isSolved()")
	if (solution == nullptr) return isSudokuSolution(size, values);
	for (unsigned cell = 0; cell < sizeSquared; cell++)
		if (values[cell] != solution[cell]) return false;
	return true;
}

bool Puzzle::matchesSolution() const {
	DEBUG_OUTPUT("Puzzle::matchesSolution()")
	// Returns true if all nonempty cells match provided solution
	// Currently returns false if no solution is provided
	if (solution == nullptr) return false;
	for (unsigned cell = 0; cell < sizeSquared; cell++)
		if (values[cell] != solution[cell] && values[cell] != 0) return false;
	return true;
}

bool Puzzle::hasConflictInRow(unsigned char row) const {
	DEBUG_OUTPUT("Puzzle::hasConflictInRow(%d)", row)
	unsigned rowOffset = row * size;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = rowOffset; cell < rowOffset + size; cell++)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) return true;
	return false;
}

bool Puzzle::hasConflictInCol(unsigned char col) const {
	DEBUG_OUTPUT("Puzzle::hasConflictInCol(%d)", col)
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = col; cell < sizeSquared; cell += size)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) return true;
	return false;
}

bool Puzzle::hasConflictInBox(unsigned char majorRow, unsigned char majorCol) const {
	DEBUG_OUTPUT("Puzzle::hasConflictInBox(%d, %d)", majorRow, majorCol)
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
	DEBUG_OUTPUT("Puzzle::hasConflictAt(%d)", cell)
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
	DEBUG_OUTPUT("Puzzle::numConflictsInRow(%d)", row)
	unsigned conflicts = 0;
	unsigned rowOffset = row * size;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = rowOffset; cell < rowOffset + size; cell++)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflictsInCol(unsigned char col) const {
	DEBUG_OUTPUT("Puzzle::numConflictsInCol(%d)", col)
	unsigned conflicts = 0;
	unsigned char vals[size+1] = {0};
	for (unsigned char cell = col; cell < sizeSquared; cell += size)
		if (vals[values[cell]]++ > 0 && values[cell] != 0) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflictsInBox(unsigned char majorRow, unsigned char majorCol) const {
	DEBUG_OUTPUT("Puzzle::numConflictsInBox(%d, %d)", majorRow, majorCol)
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
	DEBUG_OUTPUT("Puzzle::numConflictsAt(%d)", cell)
	unsigned value = this->values[cell], conflicts = 0;
	unsigned *neighbor = this->neighbors[cell];
	unsigned *neighborCeiling = neighbor + this->computeNeighborhoodSize();
	for ( ; neighbor < neighborCeiling; neighbor++)
		if (*neighbor == value) conflicts++;
	return conflicts;
}

unsigned Puzzle::numConflicts() const {
	DEBUG_OUTPUT("Puzzle::numConflicts()")
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
	DEBUG_OUTPUT("Puzzle::swap()")
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
	DEBUG_FUNC_HEADER("Puzzle::~Puzzle()")

	DEBUG_OUTPUT("Puzzle not yet initialized... returning early")
	if (this->values == nullptr) return;

	DEBUG_OUTPUT("Deleting values and concrete arrays")
	delete[] this->values;
	delete[] this->concrete;

	DEBUG_OUTPUT_IF(this->solution != nullptr, "Deleting solution") 
	if (this->solution != nullptr) delete[] this->solution;

	DEBUG_OUTPUT("Deleting Neighbors")
	for (unsigned **i = this->neighbors, **iMax = i + this->sizeSquared; i < iMax; i++) delete[] *i;
	delete[] this->neighbors;

	DEBUG_OUTPUT("Deleting Neighborhoods")
	for (unsigned ***i = this->neighborhoods, ***iMax = i + this->sizeSquared; i < iMax; i++) {
		for (unsigned **j = *i, **jMax = j + 3; j < jMax; j++) {
			delete[] *j;
		}
		delete[] *i;
	}
	delete[] this->neighborhoods;
}