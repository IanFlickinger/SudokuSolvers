#include "benchmarking.h"

/****************************************************************************************/
// SET_LOADER(PuzzleLoader("../4x4.csv", 1000000UL, 4));
SET_LOADER(PuzzleLoader("../9x9.csv", 1000000UL, 9));
/****************************************************************************************/
ADD_SOLVER(Solvers::DepthFirstSolver(), DepthFirst)
ADD_SOLVER(Solvers::DepthFirstSolverV1(), DepthFirstV1)
// ADD_SOLVER(Solvers::GeometricAnnealingSolver(20, 50, 1500, 0.95), GeometricAnnealing)
ADD_SOLVER(Solvers::AdditiveGraphSolver(100), AdditiveGraph)
ADD_SOLVER(Solvers::SimpleAdditiveGraphSolver(100), SimpleAdditiveGraph)
ADD_SOLVER(Solvers::MultiplicativeGraphSolver(100), MultiplicativeGraph)
/****************************************************************************************/

int main(int argc, char **argv) {
	RUN_BENCHMARKS(argc, argv);
	return 0;
}