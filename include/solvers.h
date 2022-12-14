#ifndef SUDOKU_SOLVER_BASIC_H
#define SUDOKU_SOLVER_BASIC_H

#include "puzzle.h"

#define SOLVER_BODY : Solver { \
    public: \
        void solve(Puzzle&) override; \
};

namespace Solvers {

class Solver {
    public: 
        virtual void solve(Puzzle&) = 0;
        Puzzle solveCopy(const Puzzle &puzzle) 
            {Puzzle newPuzzle(puzzle); solve(newPuzzle); return newPuzzle;}
};

class DepthFirstSolverV1 : public virtual Solver 
    { public: void solve(Puzzle&) override; };
class DepthFirstSolver : public virtual Solver 
    { public: void solve(Puzzle&) override; };

class AnnealingSolver : public virtual Solver {
    protected:
        unsigned iterations;
        unsigned reheats;
    public:
        AnnealingSolver(unsigned reheats, unsigned iterations);
        void solve(Puzzle&) override;
        virtual double tempSchedule(unsigned iteration, double temperature) = 0;
};
class GeometricAnnealingSolver : public virtual AnnealingSolver {
    private:
        double tempInit, tempFact;
    public: 
        GeometricAnnealingSolver(unsigned reheats, unsigned iterations, double initialTemp, double tempFactor);
        inline double tempSchedule(unsigned iteration, double temperature) override;
};

class GraphSolver : public virtual Solver {
    protected: const unsigned maxIters;
    public: 
        GraphSolver() : maxIters(100UL) {};
        GraphSolver(unsigned iters) : maxIters(iters) {};
};
class SimpleAdditiveGraphSolver : public virtual GraphSolver
    { public: void solve(Puzzle&) override; };
class AdditiveGraphSolver : public virtual GraphSolver
    { public: void solve(Puzzle&) override; };
class MultiplicativeGraphSolver : public virtual GraphSolver
    { public: void solve(Puzzle&) override; };

}

#endif