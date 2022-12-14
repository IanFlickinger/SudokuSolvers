#include "puzzle.h"

#include "graph.h"

// #define DEBUG_ENABLED
// #define DEBUG_ENABLED_VERBOSE 
#include "debugging.h"

template <typename Key, typename Val> 
struct node_t {
	Key key;
	Val val;
    void (*valDestructor)(void*, Val);
    void *valDestructorArgs;
	node_t *next = nullptr;

	node_t() = default;
	node_t(Key key, Val val) { this->key = key; this->val = val; }
    node_t(Key key, Val val, void (*valDestructor)(void*, Val), void * valDestructorArgs) {
        this->key = key;
        this->val = val;
        this->valDestructor = valDestructor;
        this->valDestructorArgs = valDestructorArgs;
    }
    ~node_t() { 
        DEBUG_OUTPUT("~node_t()") 
        this->valDestructor(valDestructorArgs, this->val); 
    }
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

    bool add(Key key, Val val, void (*valDestructor)(void*, Val), void *valDestructorArgs) {
		if (listHead == nullptr) {
			listHead = new node_t<Key, Val>(key, val, valDestructor, valDestructorArgs);
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
        DEBUG_OUTPUT("~cache_t()")
		if (listHead == nullptr) return;
		node_t<Key, Val> * cursor = listHead, *last = cursor;
		while (cursor != nullptr) {
			cursor = cursor->next;
			delete last;
			last = cursor;
		}
	}
};

struct utpArgs {
    unsigned size1, size2;
    utpArgs(unsigned size1, unsigned size2) { this->size1 = size1; this->size2 = size2; }
};

void udpDestructor(unsigned *size, unsigned **val) {
    DEBUG_OUTPUT("udpDestructor(size=%d, unsigned **)", *size)
    for (unsigned **cursor = val, **cursorMax = cursor + *size; cursor < cursorMax; cursor++)
        delete[] *cursor;
    delete[] val;
    delete size;
}

void utpDestructor(utpArgs* args, unsigned *** val) {
    DEBUG_OUTPUT("utpDestructor({size1=%d, size2=%d}, unsigned ***)", args->size1, args->size2)
    for (unsigned ***cursor1 = val, ***cursor1Max = cursor1 + args->size1; cursor1 < cursor1Max; cursor1++) {
        for (unsigned **cursor2 = *cursor1, **cursor2Max = cursor2 + args->size2; cursor2 < cursor2Max; cursor2++) 
            delete[] *cursor2;
        delete[] *cursor1;
    }
    delete[] val;
    delete args;
}

unsigned *** graphNeighborhoodByCell(unsigned size) {
    DEBUG_FUNC_HEADER("graphNeighborhoodByCell(%d)", size)
    static cache_t<unsigned, unsigned ***> cache;

    // check cache for precomputed graph structure
    unsigned *** cachedValue = cache.get(size, nullptr);
    if (cachedValue != nullptr) {
        DEBUG_OUTPUT("Retrieved value from cache")
        DEBUG_FUNC_END()
        return cachedValue;
    }

    // compute useful constants
    const unsigned sizeSquared = size * size;
    const unsigned sizeSqrt = perfectSqrt(size);
    const unsigned numNeighborhoods = 3;

	// BUILD NEIGHBORHOODS
	// TODO: several optimizations are possible
	DEBUG_OUTPUT("Building neighborhoods")
    

	unsigned neighborhoodSize = size - 1;
	unsigned ***neighborhoods = new unsigned**[sizeSquared];
	for (unsigned ***i = neighborhoods, ***iMax = i + sizeSquared; i < iMax; i++) {
		*i = new unsigned*[numNeighborhoods]; 
		for (unsigned **j = *i, **jMax = j + numNeighborhoods; j < jMax; j++) 
			*j = new unsigned[neighborhoodSize];
	}

	DEBUG_OUTPUT("Memory Allocated")

	unsigned cell = 0, rowOffset = 0, rowCeiling = size;
	unsigned boxOffset = 0, boxCeiling = (sizeSqrt - 1) * size + sizeSqrt;
	for (unsigned char row = 0, col = 0, majorRow = 0, majorCol = 0, minorRow = 0, minorCol = 0; 
		cell < sizeSquared; cell++
	) {
		DEBUG_OUTPUT("Cell %d:", cell)
		DEBUG_INDENT()

		// build row neighborhood
		for (unsigned otherCell = rowOffset, i = 0; otherCell < rowCeiling; otherCell++)
			if (otherCell != cell) neighborhoods[cell][0][i++] = otherCell;
		DEBUG_OUTPUT("Row neighborhood complete")

		// build col neighborhood
		for (unsigned otherCell = col, i = 0; otherCell < sizeSquared; otherCell += size)
			if (otherCell != cell) neighborhoods[cell][1][i++] = otherCell;
		DEBUG_OUTPUT("Column neighborhood complete")
		
		// build box neighborhood
		unsigned char otherMinorRow = 0, otherMinorCol = 0;
		for (unsigned otherCell = boxOffset, i = 0; 
				otherMinorRow < sizeSqrt; 
				otherMinorCol++, otherCell++
		) {
			if (otherMinorCol == sizeSqrt) {
				otherMinorCol = 0; otherMinorRow++;
				otherCell += size - sizeSqrt;
			}
			if (otherCell != cell) {
				neighborhoods[cell][2][i++] = otherCell;
			}
		}
		DEBUG_OUTPUT("Box neighborhood complete")
		DEBUG_OUTDENT()

		// update loop variants
		minorCol++; col++;
		if (minorCol == sizeSqrt) {
			minorCol = 0; majorCol++;
			boxOffset += sizeSqrt;
			boxCeiling += sizeSqrt;
			if (col == size) {
				col = majorCol = 0;
				minorRow++; row++;
				rowOffset += size;
				rowCeiling += size;
				boxOffset -= size;
				boxCeiling -= size;
				if (minorRow == sizeSqrt) {
					minorRow = 0; majorRow++;
					unsigned majorSize = sizeSqrt * size;
					boxOffset += majorSize;
					boxCeiling += majorSize;
				}
			}
		}
	}

    DEBUG_OUTPUT("Neighborhood list complete... Adding to cache")
    cache.add(size, neighborhoods, (void (*)(void *, unsigned ***)) &utpDestructor, (void *) new utpArgs(sizeSquared, numNeighborhoods));
    return neighborhoods;
}

unsigned *** graphNeighborhoods(unsigned size) {
    DEBUG_FUNC_HEADER("graphNeighborhoods(%d)", size)
    static cache_t<unsigned, unsigned ***> cache;

    // check cache for precomputed graph structure
    unsigned ***cachedValue = cache.get(size, nullptr);
    if (cachedValue != nullptr) {
        DEBUG_OUTPUT("Retrieved value from cache")
        DEBUG_FUNC_END()
        return cachedValue;
    }

    // compute useful constants
    const unsigned sizeSquared = size * size;
    const unsigned sizeSqrt = perfectSqrt(size);
    const unsigned numNeighborhoods = 3;

	// BUILD NEIGHBORHOODS
	DEBUG_OUTPUT("Building neighborhoods")

    // allocate memory
	unsigned neighborhoodSize = size - 1;
	unsigned ***neighborhoods = new unsigned**[numNeighborhoods];
	for (unsigned ***i = neighborhoods, ***iMax = i + numNeighborhoods; i < iMax; i++) {
		*i = new unsigned*[size]; 
		for (unsigned **j = *i, **jMax = j + size; j < jMax; j++) 
			*j = new unsigned[neighborhoodSize];
	}

	DEBUG_OUTPUT("Memory Allocated")

    // build row neighborhoods
    for (unsigned cell = 0, col = 0, row = 0, **rowPtr = neighborhoods[0], *cursor = *rowPtr; row < size; ) {
        *cursor = cell;
        cell++; col++; cursor++;
        if (col == size) {
            col = 0;
            row++; rowPtr++;
            cursor = *rowPtr;
        }
    }

    DEBUG_OUTPUT("Rows Built")
    
    // build column neighborhoods
    for (unsigned cell = 0, col = 0, row = 0, **colPtr = neighborhoods[1], *cursor = *colPtr; col < size; ) {
        *cursor = cell;
        cell += size;
        row++; cursor++;
        if (row == size) {
            row = 0;
            col++; colPtr++;
            cursor = *colPtr;
            cell = col;
        }
    }  

    DEBUG_OUTPUT("Columns Built")

    // build box neighborhoods
    unsigned minorRowUpdate = size - sizeSqrt;
    unsigned majorSize = size * sizeSqrt;
    unsigned majorColUpdate = majorSize - sizeSqrt;
    unsigned majorRowUpdate = majorSize - size;
    for (unsigned cell = 0, minorRow = 0, minorCol = 0, majorRow = 0, majorCol = 0, **boxPtr = neighborhoods[2], *cursor = *boxPtr; majorRow < sizeSqrt; ) {
        *cursor = cell;
        cell++; minorCol++; cursor++;
        if (minorCol == sizeSqrt) {
            minorCol = 0; minorRow++;
            cell += minorRowUpdate;
            if (minorRow == sizeSqrt) {
                minorRow = 0; majorCol++;
                boxPtr++; cursor = *boxPtr;
                cell -= majorColUpdate;
                if (majorCol == sizeSqrt) {
                    majorCol = 0; majorRow++;
                    cell += majorRowUpdate;
                }
            }
        }
    }

    DEBUG_OUTPUT("Boxes Built")

    cache.add(size, neighborhoods, (void (*)(void*, unsigned***)) &utpDestructor, (void *) new utpArgs(numNeighborhoods, size));

    DEBUG_OUTPUT("Neighborhood list added to cache")

    DEBUG_FUNC_END()
    return neighborhoods;
}

unsigned ** graphNeighborsByCell(unsigned size) {
    DEBUG_FUNC_HEADER("graphNeighborsByCell(%d)", size)
    static cache_t<unsigned, unsigned **> cache;

    // check cache for precomputed graph structure
    unsigned ** cachedValue = cache.get(size, nullptr);
    if (cachedValue != nullptr) {
        DEBUG_OUTPUT("Retrieved value from cache")
        DEBUG_FUNC_END()
        return cachedValue;
    }

    // compute useful constants
    const unsigned sizeSquared = size * size;
    const unsigned sizeSqrt = perfectSqrt(size);
    const unsigned numNeighborhoods = 3;
    const unsigned neighborhoodSize = numNeighborhoods * (size - 1) - (numNeighborhoods - 1) * (sizeSqrt - 1);

	// BUILD NEIGHBORHOODS
	DEBUG_OUTPUT("Building neighborhoods")
    
	unsigned **neighborhoods = new unsigned*[sizeSquared];
	for (unsigned **i = neighborhoods, **iMax = i + sizeSquared; i < iMax; i++)
		*i = new unsigned[neighborhoodSize]; 

	DEBUG_OUTPUT("Memory Allocated")

	unsigned cell = 0, rowOffset = 0, rowCeiling = size;
	unsigned boxOffset = 0, boxCeiling = (sizeSqrt - 1) * size + sizeSqrt;
	for (unsigned char row = 0, col = 0, majorRow = 0, majorCol = 0, minorRow = 0, minorCol = 0; 
		cell < sizeSquared; cell++
	) {
		DEBUG_OUTPUT("Cell %d:", cell)
		DEBUG_INDENT()
		unsigned neighborOffset = 0;

		// build row neighborhood
		for (unsigned otherCell = rowOffset, i = 0; otherCell < rowCeiling; otherCell++)
			if (otherCell != cell) neighborhoods[cell][neighborOffset++] = otherCell;
		DEBUG_OUTPUT("Row neighborhood complete")

		// build col neighborhood
		for (unsigned otherCell = col, i = 0; otherCell < sizeSquared; otherCell += size)
			if (otherCell != cell) neighborhoods[cell][neighborOffset++] = otherCell;
		DEBUG_OUTPUT("Column neighborhood complete")
		
		// build box neighborhood
		unsigned char otherMinorRow = 0, otherMinorCol = 0;
		for (unsigned otherCell = boxOffset, i = 0; 
				otherMinorRow < sizeSqrt; 
				otherMinorCol++, otherCell++
		) {
			if (otherMinorCol == sizeSqrt) {
				otherMinorCol = 0; otherMinorRow++;
				otherCell += size - sizeSqrt;
			}
			if (otherMinorCol != minorCol && otherMinorRow != minorRow) {
				neighborhoods[cell][neighborOffset++] = otherCell;
			}
		}
		DEBUG_OUTPUT("Box neighborhood complete")
		DEBUG_OUTDENT()

		// update loop variants
		minorCol++; col++;
		if (minorCol == sizeSqrt) {
			minorCol = 0; majorCol++;
			boxOffset += sizeSqrt;
			boxCeiling += sizeSqrt;
			if (col == size) {
				col = majorCol = 0;
				minorRow++; row++;
				rowOffset += size;
				rowCeiling += size;
				boxOffset -= size;
				boxCeiling -= size;
				if (minorRow == sizeSqrt) {
					minorRow = 0; majorRow++;
					unsigned majorSize = sizeSqrt * size;
					boxOffset += majorSize;
					boxCeiling += majorSize;
				}
			}
		}
	}

    DEBUG_OUTPUT("Neighborhood list complete... Adding to cache")
    cache.add(size, neighborhoods, (void (*)(void *, unsigned **)) &udpDestructor, (void *) new unsigned(sizeSquared));
    return neighborhoods;
}