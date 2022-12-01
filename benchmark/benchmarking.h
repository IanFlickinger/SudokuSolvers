#ifndef SUDOKU_BENCHMARKING_H
#define SUDOKU_BENCHMARKING_H

#include <solvers.h>
#include <string>

#include "data.h"

void RUN_BENCHMARKS(int argc, char **argv);

class BenchmarkPuzzleLoader {
    private:
        const PuzzleLoader * loader;
    public:
        static BenchmarkPuzzleLoader * GetInstance();

        BenchmarkPuzzleLoader() {};
        void setLoader(const PuzzleLoader &loader) { this->loader = &loader; }
        const PuzzleLoader &getLoader() const { return *this->loader; }
};


class SolverList {
    private:
        Solvers::Solver **solvers;
        std::string *names;

        const unsigned capinc;
        unsigned curcap;
        unsigned maxcap;

    public:
        static const unsigned DEFAULT_CAPACITY = 5;

        static SolverList * GetInstance();

        SolverList(unsigned maxcap = DEFAULT_CAPACITY);
        ~SolverList();

        int addSolver(Solvers::Solver *solver) { return addSolver(solver, std::to_string(curcap+1)); }
        int addSolver(Solvers::Solver *solver, std::string name);

        Solvers::Solver ** getSolvers() const { return solvers; };
        unsigned getNumSolvers() const { return curcap; }
        std::string * getSolverNames() const { return names; }
};


#define SET_LOADER(loader) struct LoaderDummyClass_ ## loader ## _t { \
    static int dummy; \
    static int registerLoader() { BenchmarkPuzzleLoader::GetInstance()->setLoader(loader); return 0; } \
}; \
int LoaderDummyClass_ ## loader ## _t::dummy = LoaderDummyClass_ ## loader ## _t::registerLoader();

#define SUDOKU_PUZZLE_LOADER BenchmarkPuzzleLoader::GetInstance()->getLoader()

#define ADD_SOLVER(solverBase, name) struct SolverAddition_ ## name ## _t { \
        static int dummy; \
        static int registerSolver() {SolverList::GetInstance()->addSolver(new solverBase, #name); return 0;} \
}; \
int SolverAddition_ ## name ## _t::dummy = SolverAddition_ ## name ## _t::registerSolver();

#define SOLVERS SolverList::GetInstance()->getSolvers()

#define NUM_SOLVERS SolverList::GetInstance()->getNumSolvers()

#define SOLVER_NAMES SolverList::GetInstance()->getSolverNames()

#endif // SUDOKU_BENCHMARKING_H