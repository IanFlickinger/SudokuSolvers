#include "solvers.h"
#include "puzzle.h"
#include "graph.h"


// #define DEBUG_ENABLED
// #define DEBUG_ENABLED_VERBOSE
#include "debugging.h"

using namespace Solvers;

#define COLLAPSE_THRESHOLD 0.95

typedef struct simplex_data_t {
    double *position;
    const double *positionEnd;
    unsigned char ndims;
    unsigned char value;

    simplex_data_t() = default;
    void init(unsigned char ndims) {
        this->value = 0;
        this->ndims = ndims;
        this->position = new double[ndims];
        this->positionEnd = position + ndims;
    }
    simplex_data_t(const simplex_data_t &other) {
        this->value = other.value;
        this->ndims = other.ndims;
        this->position = new double[ndims];
        this->positionEnd = this->position + this->ndims;
        for (double * val = position, *otherVal = other.position, dim = 0; dim < ndims; dim++, val++, otherVal++)
            *val = *otherVal;
    }
    inline simplex_data_t(unsigned char ndims, double initVal) 
        { init(ndims, initVal); }
    inline simplex_data_t(unsigned char ndims)
        { init(ndims, 1. / ndims); }
    inline simplex_data_t(unsigned char ndims, double *vals) {
        this->init(ndims);
        for (double *val = position, *ival = vals; val < positionEnd; val++, ival++) *val = *ival;
    }
    inline void init(unsigned char ndims, double initVal) {
        this->init(ndims);
        for (double *val = position; val < positionEnd; val++) *val = initVal;
    }

    simplex_data_t &operator-=(const simplex_data_t &other) {
        for (double *a = position, *b = other.position; a < positionEnd; a++, b++) *a -= *b;
        return *this;
    }
    simplex_data_t &operator+=(const simplex_data_t &other) {
        for (double *a = position, *b = other.position; a < positionEnd; a++, b++) *a += *b;
        return *this;
    }
    simplex_data_t &operator*=(const simplex_data_t &other) {
        for (double *a = position, *b = other.position; a < positionEnd; a++, b++) *a *= *b;
        return *this;
    }
    simplex_data_t &operator*=(double factor) {
        for (double *a = position; a < positionEnd; a++) *a *= factor;
        return *this;
    }
    simplex_data_t &operator=(const simplex_data_t &other) {
        this->value = other.value;
        for (double *a = position, *b = other.position; a < positionEnd; a++, b++) *a = *b;
        return *this;
    }
    void swap(simplex_data_t &other) {
        std::swap(position, other.position);
        std::swap(positionEnd, other.positionEnd);
        std::swap(value, other.value);
        std::swap(ndims, other.ndims);
    }
    
    void constrainSimplex() {
        bool collapsing = true;
        while (collapsing) {
            collapsing = false;
            double sum = -1; unsigned cnt = 0;
            for (double *val = position; val < positionEnd; val++) {
                if (*val > 0) { sum += *val; cnt++; }
                else { *val = 0; }
            }
            for (double *val = position, update = sum / cnt; val < positionEnd; val++) {
                if (*val > 0) *val -= update;
                if (*val < 0) collapsing = true;
            }
        }
    }
    void constrainSimplexV5() {
        bool collapsing = true;
        while (collapsing) {
            collapsing = false;
            double sum = -1; unsigned cnt = 0;
            for (double *val = position; val < positionEnd; val++) {
                if (*val > 0) { sum += *val; cnt++; }
                else { *val = 0; }
            }
            for (double *val = position, update = sum / cnt; val < positionEnd; val++) {
                if (*val > 0) *val -= update;
                if (*val < 0) collapsing = true;
            }
        }
        for (double *val = position; val < positionEnd; val++) if (*val > 0) return;
        double barycenterVal = 1. / this->ndims;
        for (double *val = position; val < positionEnd; val++) *val = barycenterVal;
    }
    
    bool collapse() {
        // if (this->value && this->position[this->value - 1] > COLLAPSE_THRESHOLD) return 0;
        for (double *val = position, dim = 0; dim < ndims; dim++, val++) 
            if (*val > COLLAPSE_THRESHOLD) {
                this->collapseTo(dim+1);
                break;
            }
        return this->value;
    }
    void collapseTo(unsigned char value) {
        for (double *val = position; val < positionEnd; val++) *val = 0;
        this->position[value-1] = 1;
        this->value = value;
    }

    ~simplex_data_t() {
        delete[] position;
    }
} simplex_data_t;

namespace std {
    string to_string(const simplex_data_t &data) {
        string str = '<' + to_string(*data.position);
        for (double *val = data.position+1; val < data.positionEnd; val++) str += ' ' + to_string(*val);
        return str + '>';
    }
}

void MultiplicativeGraphSolver::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("CollapsingGraphSolverV5::solve(Puzzle &puzzle)")

    const unsigned numNeighborhoods = 3;

    // get precomputed adjacency structures
    unsigned ***neighborhoodList = graphNeighborhoods(puzzle.getSize());
    unsigned ***neighborhoodListMax = neighborhoodList + numNeighborhoods;
    unsigned neighborhoodSize = puzzle.getSize() - 1;

    DEBUG_OUTPUT("Initializing simplex data")
    DEBUG_INDENT()

    // calculate value for barycenter coordinates
    const double simplexInitVal = 1. / puzzle.getSize();

    // initialize simplex data 
    simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
    unsigned cell = 0;
    for (simplex_data_t *dataCursor = data, *updateCursor = update; 
        dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
    ) {
        // initialize to barycenter
        dataCursor->init(puzzle.getSize(), simplexInitVal);

        // collapse if predetermined
        if (puzzle.isConcrete(cell)) {
            unsigned char value = puzzle.getValue(cell);
            dataCursor->collapseTo(value);
        }

        // initialize update to ones 
        updateCursor->init(puzzle.getSize(), 1.);
        DEBUG_OUTPUT("Simplex data for cell %d: %s", cell, std::to_string(*dataCursor).c_str())
    }
    DEBUG_OUTDENT()

    // define useful constants
    const simplex_data_t UPDATE_INIT(puzzle.getSize(), 1 - simplexInitVal); // adds barycenter for each neighbor displacement vector
    const simplex_data_t ONES(puzzle.getSize(), 1.);
    const double SCALE_FACTOR = 1. / numNeighborhoods; // 1 / number of neighborhoods (1/3 Assumes 2D puzzle)

    DEBUG_OUTPUT("Beginning graph collapse procedure")
    unsigned iteration = 0;
    while(!puzzle.isSolved() && iteration++ < this->maxIters) {
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute update vectors
        DEBUG_OUTPUT("Iteration %d: Computing update vectors", iteration)
        DEBUG_STATEMENT(std::string nType = "row")
        DEBUG_INDENT()
        for (unsigned ***neighborhoodListCursor = neighborhoodList; neighborhoodListCursor < neighborhoodListMax; neighborhoodListCursor++) {
            DEBUG_STATEMENT(unsigned n = 0)
            for (unsigned **neighborhoodCursor = *neighborhoodListCursor, **neighborhoodCursorMax = neighborhoodCursor + puzzle.getSize();
                neighborhoodCursor < neighborhoodCursorMax; neighborhoodCursor++
            ) {
                simplex_data_t updateBase(UPDATE_INIT);
                unsigned *neighborMax = *neighborhoodCursor + puzzle.getSize();
                for (unsigned *neighbor = *neighborhoodCursor; neighbor < neighborMax; neighbor++) {
                    updateBase -= data[*neighbor];
                }
                DEBUG_OUTPUT("Computed %s %d update: %s", nType.c_str(), n++, std::to_string(updateBase).c_str())
                // updateBase *= SCALE_FACTOR;
                DEBUG_STATEMENT(unsigned member = 0)
                DEBUG_INDENT()
                for (unsigned *neighbor = *neighborhoodCursor; neighbor < neighborMax; neighbor++) {
                    simplex_data_t temp(updateBase);
                    temp += data[*neighbor];
                    temp.constrainSimplexV5();
                    DEBUG_OUTPUT("Member %d perspective: %s", member++, std::to_string(temp).c_str())
                    update[*neighbor] *= temp;
                }
                DEBUG_OUTDENT()
            }
            DEBUG_STATEMENT(if (*nType.c_str() == 'r') nType = "column")
            DEBUG_STATEMENT(else nType = "box")
        }
        DEBUG_OUTDENT()

        // update state
        DEBUG_OUTPUT("Iteration %d: Updating states", iteration)
        DEBUG_INDENT()
        // iterate over all cells
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            // ignore concrete cells
            if (puzzle.isConcrete(cell)) continue;

            DEBUG_OUTPUT("Updating Cell %d", cell)
            DEBUG_INDENT()

            // add update vectors and reconstrain to simplex
            *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();

            DEBUG_OUTPUT("Update vector: %s", std::to_string(*updateCursor).c_str())
            DEBUG_OUTPUT("Resultant vector: %s", std::to_string(*dataCursor).c_str())
            
            // check for node collapse
            if (dataCursor->collapse()) {
                DEBUG_OUTPUT("Collapsing cell at row %d and column %d to %d", CELL_TO_COORDS(cell, puzzle.getSize()), dataCursor->value)
                puzzle.setValue(cell, dataCursor->value);
            }

            DEBUG_OUTDENT()

            // reset update cursor 
            *updateCursor = ONES;
        }
        DEBUG_OUTDENT()
    }
    
    // free heap memory
    DEBUG_OUTPUT("Puzzle solved")
    delete[] data;
    delete[] update;

    DEBUG_OUTPUT("Heap memory freed")
    DEBUG_FUNC_END()
}

void SimpleAdditiveGraphSolver::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("CollapsingGraphSolverV4::solve(Puzzle &puzzle)")

    const unsigned numNeighborhoods = 3;

    // get precomputed adjacency structures
    unsigned **neighborsList = graphNeighborsByCell(puzzle.getSize());
    unsigned **neighborsListMax = neighborsList + puzzle.getSizeSquared();
    // unsigned neighborhoodSize = puzzle.getSize() - 1;
    unsigned neighborhoodSize = puzzle.computeNeighborhoodSize();

    DEBUG_OUTPUT("Initializing simplex data")
    DEBUG_INDENT()

    // calculate value for barycenter coordinates
    const double simplexInitVal = 1. / puzzle.getSize();

    // initialize simplex data 
    simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
    unsigned cell = 0;
    for (simplex_data_t *dataCursor = data, *updateCursor = update; 
        dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
    ) {
        // initialize to barycenter
        dataCursor->init(puzzle.getSize(), simplexInitVal);

        // collapse if predetermined
        if (puzzle.isConcrete(cell)) {
            unsigned char value = puzzle.getValue(cell);
            dataCursor->collapseTo(value);
        }

        // allocate memory, ignore values
        updateCursor->init(puzzle.getSize());
        DEBUG_OUTPUT("Simplex data for cell %d: %s", cell, std::to_string(*dataCursor).c_str())
    }
    DEBUG_OUTDENT()

    // define useful constants
    const simplex_data_t UPDATE_INIT(puzzle.getSize(), simplexInitVal * neighborhoodSize); // adds barycenter for each neighbor displacement vector
    const double SCALE_FACTOR = 1. / neighborhoodSize; 
    

    DEBUG_OUTPUT("Beginning graph collapse procedure")
    unsigned iteration = 0;
    while(!puzzle.isSolved() && iteration++ < this->maxIters) {
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute update vectors
        DEBUG_OUTPUT("Iteration %d: Computing update vectors", iteration)
        DEBUG_INDENT()
        for (unsigned **neighbors = neighborsList, cell = 0; neighbors < neighborsListMax; neighbors++, dataCursor++, updateCursor++, cell++) {
            if (puzzle.isConcrete(cell)) continue;
            *updateCursor = UPDATE_INIT;
            for (unsigned *neighbor = *neighbors, *neighborMax = neighbor + neighborhoodSize; neighbor < neighborMax; neighbor++)
                *updateCursor -= data[*neighbor];
            *updateCursor *= SCALE_FACTOR;
        }
        DEBUG_OUTDENT()

        // update state
        DEBUG_OUTPUT("Iteration %d: Updating states", iteration)
        DEBUG_INDENT()
        // iterate over all cells
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            // ignore concrete cells
            if (puzzle.isConcrete(cell)) continue;

            // add update vectors and reconstrain to simplex
            *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();
            DEBUG_STATEMENT(simplex_data_t mem(*dataCursor))
            
            // check for node collapse
            if (dataCursor->collapse()) {
                DEBUG_OUTPUT("Collapsing cell at row %d and column %d to %d from %s", CELL_TO_COORDS(cell, puzzle.getSize()), dataCursor->value, std::to_string(mem).c_str())
                puzzle.setValue(cell, dataCursor->value);
            }
        }
        DEBUG_OUTDENT()
        
    }
    
    // free heap memory
    DEBUG_OUTPUT("Puzzle solved")
    delete[] data;
    delete[] update;

    DEBUG_OUTPUT("Heap memory freed")
    DEBUG_FUNC_END()
}

void AdditiveGraphSolver::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("CollapsingGraphSolver::solve(Puzzle &puzzle)")

    const unsigned numNeighborhoods = 3;

    // get precomputed adjacency structures
    unsigned ***neighborhoodList = graphNeighborhoods(puzzle.getSize());
    unsigned ***neighborhoodListMax = neighborhoodList + numNeighborhoods;
    unsigned neighborhoodSize = puzzle.getSize() - 1;

    DEBUG_OUTPUT("Initializing simplex data")
    DEBUG_INDENT()

    // calculate value for barycenter coordinates
    const double simplexInitVal = 1. / puzzle.getSize();

    // initialize simplex data 
    simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
    unsigned cell = 0;
    for (simplex_data_t *dataCursor = data, *updateCursor = update; 
        dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
    ) {
        // initialize to barycenter
        dataCursor->init(puzzle.getSize(), simplexInitVal);

        // allocate memory, but ignore values
        updateCursor->init(puzzle.getSize());

        // collapse if predetermined
        if (puzzle.isConcrete(cell)) {
            unsigned char value = puzzle.getValue(cell);
            dataCursor->collapseTo(value);
        }

        // initialize update to node location
        *updateCursor = *dataCursor;
        DEBUG_OUTPUT("Simplex data for cell %d: %s", cell, std::to_string(*dataCursor).c_str())
    }
    DEBUG_OUTDENT()

    // define useful constants
    const simplex_data_t UPDATE_INIT(puzzle.getSize(), 1 - simplexInitVal); // adds barycenter for each neighbor displacement vector
    const double SCALE_FACTOR = 1. / numNeighborhoods; // 1 / number of neighborhoods (1/3 Assumes 2D puzzle)
    

    DEBUG_OUTPUT("Beginning graph collapse procedure")
    unsigned iteration = 0;
    while(!puzzle.isSolved() && iteration++ < this->maxIters) {
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute update vectors
        DEBUG_OUTPUT("Iteration %d: Computing update vectors", iteration)
        DEBUG_STATEMENT(std::string nType = "row")
        DEBUG_INDENT()
        for (unsigned ***neighborhoodListCursor = neighborhoodList; neighborhoodListCursor < neighborhoodListMax; neighborhoodListCursor++) {
            DEBUG_STATEMENT(unsigned n = 1;)
            for (unsigned **neighborhoodCursor = *neighborhoodListCursor, **neighborhoodCursorMax = neighborhoodCursor + puzzle.getSize();
                neighborhoodCursor < neighborhoodCursorMax; neighborhoodCursor++
            ) {
                DEBUG_OUTPUT("Computing %s %d update", nType.c_str(), n++)
                simplex_data_t updateBase(UPDATE_INIT);
                unsigned *neighborMax = *neighborhoodCursor + puzzle.getSize();
                for (unsigned *neighbor = *neighborhoodCursor; neighbor < neighborMax; neighbor++) {
                    updateBase -= data[*neighbor];
                }
                updateBase *= SCALE_FACTOR;
                for (unsigned *neighbor = *neighborhoodCursor; neighbor < neighborMax; neighbor++)
                    update[*neighbor] += updateBase;
            }
            DEBUG_STATEMENT(if (*nType.c_str() == 'r') nType = "column")
            DEBUG_STATEMENT(else nType = "box")
        }
        DEBUG_OUTDENT()

        // update state
        DEBUG_OUTPUT("Iteration %d: Updating states", iteration)
        DEBUG_INDENT()
        // iterate over all cells
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            // ignore concrete cells
            if (puzzle.isConcrete(cell)) continue;

            // add update vectors and reconstrain to simplex
            *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();
            DEBUG_STATEMENT(simplex_data_t mem(*dataCursor))
            
            // check for node collapse
            if (dataCursor->collapse()) {
                DEBUG_OUTPUT("Collapsing cell at row %d and column %d to %d from %s", CELL_TO_COORDS(cell, puzzle.getSize()), dataCursor->value, std::to_string(mem).c_str())
                puzzle.setValue(cell, dataCursor->value);
            }

            // reset update cursor to current location
            *updateCursor = *dataCursor;
        }
        DEBUG_OUTDENT()
    }
    
    // free heap memory
    DEBUG_OUTPUT("Puzzle solved")
    delete[] data;
    delete[] update;

    DEBUG_OUTPUT("Heap memory freed")
    DEBUG_FUNC_END()
}