#include "benchmarking.h"

/****************************************************************************************/
// SET_LOADER(SUDOKU_4X4_LOADER)
// SET_LOADER(SUDOKU_9X9_LOADER) // DEFAULT
/****************************************************************************************/
ADD_SOLVER(Solvers::DepthFirstSolver(), DepthFirst)
// ADD_SOLVER(Solvers::DepthFirstSolverV1(), DepthFirstV1)
// ADD_SOLVER(Solvers::GeometricAnnealingSolver(50, 10, 2000, 0.2), GeometricAnnealing)
// ADD_SOLVER(Solvers::CollapsingGraphSolver(), CollapsingGraph)
/****************************************************************************************/

int main(int argc, char **argv) {
	RUN_BENCHMARKS(argc, argv);
	return 0;
}