#include "solvers.h"
#include "puzzle.h"

#ifdef DEBUG_GRAPH_SOLVERS_CPP
 #include <iostream>
 #include <iomanip>
 #include <string>
#endif

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
    simplex_data_t &operator&=(const simplex_data_t &other) {
        for (double *a = position, *b = other.position; a < positionEnd; a++, b++) *a = *b ? *a : 0;
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

    void constrainAffine() {
        double sum = 0;
        for (double *val = position; val < positionEnd; val++) sum += *val;
        double update = -sum / ndims;
        for (double *val = position; val < positionEnd; val++) *val += update;
    }
    
    void constrainSimplex() {
        // double sum = 0;
        // for (double *val = position; val < positionEnd; val++) {
        //     if (*val < 0) *val = 0;
        //     else sum += *val;
        // }
        // if (sum != 0) for (double *val = position; val < positionEnd; val++) *val /= sum;
        // else for (double *val = position, value = 1. / ndims; val < positionEnd; val++) *val = value;

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
        if (this->value) return false;
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
    void forceCollapse() {
        double max = 0;
        for (double dim = 0, *val = position; dim < ndims; dim++, val++)
            if (*val > max) { max = *val, this->value = dim; }
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
    #ifdef DEBUG_GRAPH_SOLVERS_CPP
     std::cout << "CollapsingGraphSolver::solve(Puzzle &puzzle)\n" << std::endl;
    #endif
    #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
     std::cout << std::setw(55) << std::setfill('=') << "\n" << std::endl;
    #endif

    const unsigned ***neighborhoodList = puzzle.neighborhoodList();
    const unsigned ***neighborhoodListMax = neighborhoodList + puzzle.getSizeSquared();
    const unsigned neighborhoodSize = puzzle.getSize() - 1;

    #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
     std::cout << "Initializing simplex data" << std::endl;
    #endif

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
        #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
         std::cout << "\tSimplex data for cell " << cell << ": " << std::to_string(*dataCursor) << std::endl;
        #endif
    }

    // #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
    //  std::cout << "Propagating collapses to immediate neighbors" << std::endl;
    // #endif
    // cell = 0;
    // const unsigned **neighborListCursor = puzzle.neighborList();
    // for (simplex_data_t *dataCursor = data; 
    //     dataCursor < dataCursorCeiling; dataCursor++, neighborListCursor++, cell++
    // ) {
    //     if (puzzle.isConcrete(cell)) continue;
    //     for (const unsigned *neighborCursor = *neighborListCursor, *cursorMax = neighborCursor + puzzle.computeNeighborhoodSize(); 
    //     neighborCursor < cursorMax; neighborCursor++) {
    //         if (puzzle.isConcrete(*neighborCursor)) {
    //             dataCursor->position[data[*neighborCursor].value] = 0; 
    //         }
    //     }
    //     dataCursor->constrainSimplex();
    //     #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
    //      std::cout << "\tSimplex data for cell " << cell << ": " << std::to_string(*dataCursor) << std::endl;
    //     #endif
    // }

    const simplex_data_t ONES{puzzle.getSize(), 1.};
    const simplex_data_t ZEROS(puzzle.getSize(), 0.);
    const simplex_data_t CENTROID(puzzle.getSize(), simplexInitVal);
    

    #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
     std::cout << "Beginning graph collapse procedure" << std::endl;
     unsigned iteration = 1;
    #endif
    unsigned neighborhood = 0; // Used to iterate over rows, then columns, then boxes  
    while(!puzzle.isSolved()) {
        const unsigned ***neighborListCursor = neighborhoodList;
        simplex_data_t *dataCursor = data, *updateCursor = update;

        // compute next state
        #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
         std::cout << "Iteration " << iteration << ": Computing next states" << std::endl;
         unsigned debug_cell = 0;
        #endif
        for (unsigned cell = 0 ; dataCursor < dataCursorCeiling; cell++, neighborListCursor++, dataCursor++, updateCursor++) {
            #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
             debug_cell++;
            #endif
            if (puzzle.isConcrete(cell)) continue;
            #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
             std::cout << "\tComputing update for cell " << debug_cell-1 << ": "<< std::to_string(*dataCursor) << std::endl;
            #endif
            *updateCursor = CENTROID;
            for (unsigned neighborhood = 0; neighborhood < 3; neighborhood++) {
            simplex_data_t temp(ONES);
            const unsigned *neighborCursor = (*neighborListCursor)[neighborhood];
            const unsigned *neighborCursorMax = neighborCursor + neighborhoodSize;
            for ( ; neighborCursor < neighborCursorMax; neighborCursor++) {
                #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
                std::cout << "\t\tUpdate from neighboring cell " << *neighborCursor << ": " 
                          << std::to_string(data[*neighborCursor]) << std::endl;
                #endif
                temp -= data[*neighborCursor];
                // updateCursor->constrainSimplex();
            }
            *updateCursor += temp;
            *updateCursor -= CENTROID;
            }
            // updateCursor->constrainAffine();
            // updateCursor->constrainSimplex();
            // *updateCursor *= 1 / 3.;
        }

        // update state
        #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
         std::cout << "Iteration " << iteration << ": Updating states" << std::endl;
         iteration++;
        #endif
        dataCursor = data, updateCursor = update;
        for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
            if (puzzle.isConcrete(cell)) continue;
            // *dataCursor &= *updateCursor;
            dataCursor->swap(*updateCursor);
            // *dataCursor += *updateCursor;
            dataCursor->constrainSimplex();
            #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
             simplex_data_t mem(*dataCursor);
            #endif
            
            if (dataCursor->collapse()) 
            #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
            {
                std::cout << "\tCollapsing cell at row " << std::to_string(cell / puzzle.getSize())
                          << " and column " << std::to_string(cell % puzzle.getSize())
                          << " to " << std::to_string(dataCursor->value) 
                          << " from " << std::to_string(mem) << std::endl;
            #endif
                puzzle.setValue(cell, dataCursor->value);
            #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
            }
            #endif
        }

        neighborhood++;
        if (neighborhood == 3) neighborhood = 0;
    }

    // for (unsigned node = 0; node < puzzle.getSizeSquared(); node++) {
    //     delete nodeData[node].position;
    // }
    delete[] data;
    delete[] update;
}

// void CollapsingGraphSolver::solve(Puzzle &puzzle) {
//     #ifdef DEBUG_GRAPH_SOLVERS_CPP
//      std::cout << "CollapsingGraphSolver::solve(Puzzle &puzzle)\n" << std::endl;
//     #endif
//     #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//      std::cout << std::setw(55) << std::setfill('=') << "\n" << std::endl;
//     #endif

//     const unsigned **neighborList = puzzle.neighborList();
//     const unsigned **neighborListMax = neighborList + puzzle.getSizeSquared();

//     #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//      std::cout << "Initializing simplex data" << std::endl;
//     #endif

//     const double simplexInitVal = 1. / puzzle.getSize();

//     simplex_data_t *data = new simplex_data_t[puzzle.getSizeSquared()];
//     simplex_data_t *update = new simplex_data_t[puzzle.getSizeSquared()];
//     simplex_data_t *dataCursorCeiling = data + puzzle.getSizeSquared();
//     unsigned cell = 0;
//     for (simplex_data_t *dataCursor = data, *updateCursor = update; 
//         dataCursor < dataCursorCeiling; dataCursor++, updateCursor++, cell++
//     ) {
//         dataCursor->init(puzzle.getSize(), simplexInitVal);
//         updateCursor->init(puzzle.getSize(), simplexInitVal);
//         if (puzzle.isConcrete(cell)) {
//             unsigned char value = puzzle.getValue(cell);
//             dataCursor->collapseTo(value);
//         }
//         #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//          std::cout << "\tSimplex data for cell " << cell << ": " << std::to_string(*dataCursor) << std::endl;
//         #endif
//     }

//     const simplex_data_t ONES{puzzle.getSize(), 1.};
//     const simplex_data_t ZEROS(puzzle.getSize(), 0.);
    

//     #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//      std::cout << "Beginning graph collapse procedure" << std::endl;
//      unsigned iteration = 1;
//     #endif
//     while(!puzzle.isSolved()) {
//         const unsigned **neighborListCursor = neighborList;
//         simplex_data_t *dataCursor = data, *updateCursor = update;

//         // compute next state
//         #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//          std::cout << "Iteration " << iteration << ": Computing next states" << std::endl;
//          unsigned cell = 0;
//         #endif
//         for ( ; dataCursor < dataCursorCeiling; neighborListCursor++, dataCursor++, updateCursor++) {
//             #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//              cell++;
//             #endif
//             if (dataCursor->value) continue;
//             #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//              std::cout << "\tComputing update for cell " << cell-1 << ": "<< std::to_string(*dataCursor) << std::endl;
//             #endif
//             const unsigned neighborhoodSize = puzzle.computeNeighborhoodSize();
//             const unsigned *neighborCursor = *neighborListCursor;
//             const unsigned *neighborCursorMax = neighborCursor + neighborhoodSize;
//             *updateCursor = *dataCursor; 
//             *updateCursor *= neighborhoodSize;
//             for ( ; neighborCursor < neighborCursorMax; neighborCursor++) {
//                 #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//                  std::cout << "\t\tUpdate from neighboring cell " << *neighborCursor << ": " << std::to_string(data[*neighborCursor]) << std::endl;
//                 #endif
//                 *updateCursor -= data[*neighborCursor];
//             }
//             updateCursor->constrainAffine();
//             updateCursor->constrainSimplex();
//         }

//         // update state
//         #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//          std::cout << "Iteration " << iteration << ": Updating states" << std::endl;
//          iteration++;
//         #endif
//         dataCursor = data, updateCursor = update;
//         for (unsigned cell = 0; cell < puzzle.getSizeSquared(); cell++, dataCursor++, updateCursor++) {
//             if (dataCursor->value) continue;
//             dataCursor->swap(*updateCursor);
//             // *dataCursor += *updateCursor;
//             // dataCursor->constrainSimplex();
//             #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//              simplex_data_t mem(*dataCursor);
//             #endif
            
//             if (dataCursor->collapse()) 
//             #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//             {
//                 std::cout << "\tCollapsing cell at row " << std::to_string(cell / puzzle.getSize())
//                           << " and column " << std::to_string(cell % puzzle.getSize())
//                           << " to " << std::to_string(dataCursor->value) 
//                           << " from " << std::to_string(mem) << std::endl;
//             #endif
//                 puzzle.setValue(cell, dataCursor->value);
//             #ifdef DEBUG_GRAPH_SOLVERS_CPP_VERBOSE
//             }
//             #endif
//         }
//     }

//     // for (unsigned node = 0; node < puzzle.getSizeSquared(); node++) {
//     //     delete nodeData[node].position;
//     // }
//     delete[] data;
//     delete[] update;
// }