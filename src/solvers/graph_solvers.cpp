#include "solvers.h"
#include "puzzle.h"


#define DEBUG_ENABLED false
#define DEBUG_ENABLED_VERBOSE false
#include "debugging.h"

using namespace Solvers;

#define COLLAPSE_THRESHOLD 0.95

typedef struct simplex_data_t {
    double *position;
    const double *positionEnd;
    unsigned char ndims;
    unsigned char value;

    simplex_data_t() = default;
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
    simplex_data_t(unsigned char ndims, double *vals) {
        this->value = 0;
        this->ndims = ndims;
        this->position = new double[ndims];
        this->positionEnd = this->position + ndims;
        for (double *val = position, *ival = vals; val < positionEnd; val++, ival++) *val = *ival;
    }
    void init(unsigned char ndims, double initVal) {
        this->value = 0;
        this->ndims = ndims;
        this->position = new double[ndims];
        this->positionEnd = position + ndims;
        for (double *val = position; val < positionEnd; val++) *val = initVal;
    }
    inline void init(unsigned char ndims) { init(ndims, 1. / ndims); }

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

    bool collapse() {
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

void CollapsingGraphSolver::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("CollapsingGraphSolver::solve(Puzzle &puzzle)")

    const unsigned ***neighborhoodList = puzzle.neighborhoodList();
    const unsigned ***neighborhoodListMax = neighborhoodList + puzzle.getSizeSquared();
    const unsigned neighborhoodSize = puzzle.getSize() - 1;

    DEBUG_OUTPUT("Initializing simplex data")

    const double simplexInitVal = 1. / puzzle.getSize();

    simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
    unsigned cell = 0;
    for (simplex_data_t *dataCursor = data, *updateCursor = update; 
        dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
    ) {
        dataCursor->init(puzzle.getSize(), simplexInitVal);
        updateCursor->init(puzzle.getSize(), simplexInitVal);
        if (puzzle.isConcrete(cell)) {
            unsigned char value = puzzle.getValue(cell);
            dataCursor->collapseTo(value);
        }
        DEBUG_OUTPUT("Simplex data for cell %d: %s", cell, std::to_string(*dataCursor))
    }

    const simplex_data_t ONES(puzzle.getSize(), 1.);
    const simplex_data_t CENTROID(puzzle.getSize(), simplexInitVal);
    const double SCALE_FACTOR = 1. / 3; // 1 / number of neighborhoods (1/3 Assumes 2D puzzle)
    

    DEBUG_OUTPUT("Beginning graph collapse procedure")
    DEBUG_STATEMENT(unsigned iteration = 1;)
    while(!puzzle.isSolved()) {
        const unsigned ***neighborListCursor = neighborhoodList;
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute state update
        DEBUG_OUTPUT("Iteration %d: Computing next states", iteration)
        DEBUG_INDENT()
        DEBUG_STATEMENT(unsigned debug_cell = 0;)
        for (unsigned cell = 0 ; dataCursor < dataCursorCeiling; cell++, neighborListCursor++, dataCursor++, updateCursor++) {
            DEBUG_STATEMENT(debug_cell++;)
            if (puzzle.isConcrete(cell)) continue;
            DEBUG_OUTPUT("Computing update for cell %d: %s", debug_cell-1, std::to_string(*dataCursor))
            DEBUG_INDENT()
            *updateCursor = CENTROID;
            for (unsigned neighborhood = 0; neighborhood < 3; neighborhood++) {
                simplex_data_t temp(ONES);
                const unsigned *neighborCursor = (*neighborListCursor)[neighborhood];
                const unsigned *neighborCursorMax = neighborCursor + neighborhoodSize;
                for ( ; neighborCursor < neighborCursorMax; neighborCursor++) {
                    DEBUG_OUTPUT("Update from neighboring cell %d: %s", *neighborCursor, std::to_string(data[*neighborCursor]))
                    temp -= data[*neighborCursor];
                }
                *updateCursor += temp;
                *updateCursor -= CENTROID;
            }
            DEBUG_OUTDENT()
            *updateCursor *= SCALE_FACTOR;
        }
        DEBUG_OUTDENT()

        // update state
        DEBUG_OUTPUT("Iteration %d: Updating states", iteration)
        DEBUG_INDENT()
        DEBUG_STATEMENT(iteration++;)
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            if (puzzle.isConcrete(cell)) continue;
            *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();
            DEBUG_STATEMENT(simplex_data_t mem(*dataCursor);)
            
            if (dataCursor->collapse()) {
                DEBUG_OUTPUT("Collapsing cell at row %d and column %d to %d from %s", cell / puzzle.getSize(), cell % puzzle.getSize(), dataCursor->value, std::to_string(mem))
                puzzle.setValue(cell, dataCursor->value);
            }
        }
        DEBUG_OUTDENT()
    }
    
    delete[] data;
    delete[] update;
}

void CollapsingGraphSolverV1::solve(Puzzle &puzzle) {
    DEBUG_FUNC_HEADER("CollapsingGraphSolverV1::solve(Puzzle &puzzle)")

    const unsigned ***neighborhoodList = puzzle.neighborhoodList();
    const unsigned ***neighborhoodListMax = neighborhoodList + puzzle.getSizeSquared();
    const unsigned neighborhoodSize = puzzle.getSize() - 1;

    DEBUG_OUTPUT("Initializing simplex data")

    const double simplexInitVal = 1. / puzzle.getSize();

    simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
    simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
    unsigned cell = 0;
    for (simplex_data_t *dataCursor = data, *updateCursor = update; 
        dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
    ) {
        dataCursor->init(puzzle.getSize(), simplexInitVal);
        updateCursor->init(puzzle.getSize(), simplexInitVal);
        if (puzzle.isConcrete(cell)) {
            unsigned char value = puzzle.getValue(cell);
            dataCursor->collapseTo(value);
        }
        DEBUG_OUTPUT("Simplex data for cell %d: %s", cell, std::to_string(*dataCursor))
    }

    const simplex_data_t ONES(puzzle.getSize(), 1.);
    const simplex_data_t CENTROID(puzzle.getSize(), simplexInitVal);
    const double SCALE_FACTOR = 1. / 3; // 1 / number of neighborhoods (1/3 Assumes 2D puzzle)
    

    DEBUG_OUTPUT("Beginning graph collapse procedure")
    DEBUG_STATEMENT(unsigned iteration = 1)
    while(!puzzle.isSolved()) {
        const unsigned ***neighborListCursor = neighborhoodList;
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute state update
        DEBUG_OUTPUT("Iteration %d: Computing next states", iteration)
        DEBUG_INDENT()
        DEBUG_STATEMENT(unsigned debug_cell = 0)
        for (unsigned cell = 0 ; dataCursor < dataCursorCeiling; cell++, neighborListCursor++, dataCursor++, updateCursor++) {
            DEBUG_STATEMENT(debug_cell++)
            if (puzzle.isConcrete(cell)) continue;
            DEBUG_OUTPUT("Computing update for cell %d: %s", debug_cell-1, std::to_string(*dataCursor))
            DEBUG_INDENT()
            *updateCursor = CENTROID;
            for (unsigned neighborhood = 0; neighborhood < 3; neighborhood++) {
                simplex_data_t temp(ONES);
                const unsigned *neighborCursor = (*neighborListCursor)[neighborhood];
                const unsigned *neighborCursorMax = neighborCursor + neighborhoodSize;
                for ( ; neighborCursor < neighborCursorMax; neighborCursor++) {
                    DEBUG_OUTPUT("Update from neighboring cell %d: %s", *neighborCursor, std::to_string(data[*neighborCursor]))
                    temp -= data[*neighborCursor];
                }
                *updateCursor += temp;
                *updateCursor -= CENTROID;
            }
            DEBUG_OUTDENT()
            *updateCursor *= SCALE_FACTOR;
        }
        DEBUG_OUTDENT()

        // update state
        DEBUG_OUTPUT("Iteration %d: Updating states", iteration)
        DEBUG_INDENT()
        DEBUG_STATEMENT(iteration++)
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            if (puzzle.isConcrete(cell)) continue;
            *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();
            DEBUG_STATEMENT(simplex_data_t mem(*dataCursor))
            
            if (dataCursor->collapse()) {
                DEBUG_OUTPUT("Collapsing cell at row %d and column %d to %d from %s", cell / puzzle.getSize(), cell % puzzle.getSize(), dataCursor->value, std::to_string(mem))
                puzzle.setValue(cell, dataCursor->value);
            }
        }
        DEBUG_OUTDENT()
    }
    
    delete[] data;
    delete[] update;
}