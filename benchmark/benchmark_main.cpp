#include "benchmarking.h"

/****************************************************************************************/
SET_LOADER(PuzzleLoader("../9x9.csv", 1000000UL, 9));
// SET_LOADER(PuzzleLoader("../4x4.csv", 1000000UL, 4));
/****************************************************************************************/
ADD_SOLVER(Solvers::DepthFirstSolver(), DepthFirst)
ADD_SOLVER(Solvers::DepthFirstSolverV1(), DepthFirstV1)
ADD_SOLVER(Solvers::GeometricAnnealingSolver(10, 50, 2000, 0.2), GeometricAnnealing)
ADD_SOLVER(Solvers::CollapsingGraphSolver(), CollapsingGraph)
/****************************************************************************************/

int main(int argc, char **argv) {
	RUN_BENCHMARKS(argc, argv);
	return 0;
}