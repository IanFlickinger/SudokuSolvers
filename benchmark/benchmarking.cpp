#include <iostream>
#include <iomanip>
#include <chrono>

#include "puzzle.h"
#include "display.h"
#include "data.h"
#include "solvers.h"

// #define DEBUG_ENABLED
// #define DEBUG_ENABLED_VERBOSE
#include "debugging.h"

#include "benchmarking.h"

using namespace std;

BenchmarkPuzzleLoader * BenchmarkPuzzleLoader::GetInstance() {
	DEBUG_OUTPUT("BenchmarkPuzzleLoader::GetInstance()")
	static BenchmarkPuzzleLoader bpl;
	return &bpl;
}

SolverList * SolverList::GetInstance() {
	DEBUG_OUTPUT("SolverList::GetInstance()")
    static SolverList sl;
    return &sl;
}

SolverList::SolverList(unsigned maxcap) : maxcap(maxcap), capinc(maxcap), curcap(0) {
	DEBUG_OUTPUT("SolverList::SolverList(%d)", maxcap)
    solvers = new Solvers::Solver*[maxcap];
    names = new std::string[maxcap];
}

SolverList::~SolverList() {
	DEBUG_OUTPUT("SolverList::~SolverList()")
    for (unsigned i = 0; i < curcap; i++) delete solvers[i];
    delete[] solvers;
}

int SolverList::addSolver(Solvers::Solver *solver, std::string name) {
	DEBUG_FUNC_HEADER("SolverList::addSolver(Solver, %s)", name.c_str())
    if (curcap >= maxcap) {
		DEBUG_OUTPUT("Capacity overflow detected")
		DEBUG_STATEMENT(unsigned oldcap = maxcap)
        maxcap += capinc;

		DEBUG_OUTPUT("Increasing capacity from %d to %d", oldcap, maxcap)
        Solvers::Solver **solversExt = new Solvers::Solver*[maxcap];
        std::string *namesExt = new std::string[maxcap];

		DEBUG_OUTPUT("Transferring items to newly allocated memory")
        for (unsigned i = 0; i < curcap; i++) {
            solversExt[i] = solvers[i];
            namesExt[i] = names[i];
        }

		DEBUG_OUTPUT("Freeing previously allocated memory")
        delete[] solvers;
        solvers = solversExt;
		delete[] names;
		names = namesExt;
    }
	DEBUG_OUTPUT("Appending solver info to list")
    names[curcap] = name;
    solvers[curcap++] = solver;

	DEBUG_FUNC_RETURN(0)
    return 0;
}

typedef struct time_compare_t {
	const unsigned numSolvers;
	const unsigned numTests;

	unsigned * solves;
	unsigned ** durations;

	time_compare_t(unsigned numSolvers, unsigned numTests) :
		numSolvers(numSolvers), numTests(numTests)
	{
		solves = new unsigned[numSolvers];
		for (unsigned *cursor = solves, *cursorMax = solves + numSolvers; cursor < cursorMax; cursor++)
			*cursor = 0;
		durations = new unsigned*[numSolvers];
		for (unsigned **cursor = durations, **cursorMax = durations + numSolvers; cursor < cursorMax; cursor++)
			*cursor = new unsigned[numTests];
	}

	~time_compare_t() {
		delete[] solves;

		for (unsigned **cursor = durations, **cursorMax = durations + numSolvers; cursor < cursorMax; cursor++)
			delete[] *cursor;
		delete[] durations;
	}
} time_compare_t;

void compareSolvers(
	unsigned numTests, 
	unsigned numPuzzles, 
	unsigned numSolvers,
	std::string *names,
	Solvers::Solver **solvers,
	time_compare_t &timeCompare
) {
	DEBUG_FUNC_HEADER("compareSolvers(%d, %d, %d, string*, Solver**, time_compare_t&)", numTests, numPuzzles, numSolvers)
	Puzzle puzzles[numPuzzles]; 

	unsigned start, stop, duration, numSolved;
	for (unsigned testNum = 0; testNum < numTests; testNum++) {
		// Sample puzzles
		DEBUG_OUTPUT("Sampling %d Puzzles", numPuzzles)
		for (Puzzle *ptr = puzzles, *maxPtr = ptr+numPuzzles; ptr < maxPtr; ptr++) {
			Puzzle temp = SUDOKU_PUZZLE_LOADER.load();
			ptr->swap(temp);
		}

		DEBUG_IF_THEN(numPuzzles == 1, Display::showPuzzle(*puzzles))

		// Iterate over solvers
		for (unsigned solverNum = 0; solverNum < numSolvers; solverNum++) {
			Solvers::Solver &solver = *solvers[solverNum];
			std::string &name = names[solverNum];

			// Test solver 
			DEBUG_OUTPUT("Running %s solver", name.c_str())
			start = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			for (Puzzle *puzzle = puzzles, *puzzleMax = puzzles + numPuzzles; puzzle < puzzleMax; puzzle++) {
				solver.solve(*puzzle);
			}
			stop = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			
			DEBUG_IF_THEN(numPuzzles == 1, Display::showPuzzle(*puzzles))

			// Track stats
			duration = stop - start;
			timeCompare.durations[solverNum][testNum] = duration;

			numSolved = 0;
			for (unsigned i = 0; i < numPuzzles; i++) numSolved += puzzles[i].isSolved();
			timeCompare.solves[solverNum] += numSolved;

			cout << name << " solver solved " << numSolved << " out of " << numPuzzles << " puzzles in " 
				<< setprecision(5) << duration * 1e-3 << " seconds\n";

			// Reset
			DEBUG_OUTPUT("Resetting Puzzles")
			for (unsigned i = 0; i < numPuzzles; i++) puzzles[i].reset();
		}
	}
	DEBUG_FUNC_END()
}

void displayComparisonStats(
	unsigned numTests, 
	unsigned numPuzzles, 
	unsigned numSolvers, 
	std::string *names, 
	time_compare_t &timeCompare
) {
	DEBUG_OUTPUT("displayComparisonStats(%d, %d, %d, string*, time_compare_t&)", numTests, numPuzzles, numSolvers)
	// initialize means and variances
	double means[numSolvers], *meanMax = means + numSolvers;
	double vars[numSolvers], *varMax = vars + numSolvers;
	double effs[numSolvers*numSolvers], *effsMax = effs + numSolvers*numSolvers;
	for (double *mean = means, *var = vars; mean < meanMax; mean++, var++) 
		*mean = *var = 0;

	// compute means
	unsigned **durations = timeCompare.durations;
	for (double *mean = means; mean < meanMax; mean++, durations++)
		for (unsigned *duration = *durations, *durationMax = duration + numTests; duration < durationMax; duration++)
			*mean += *duration;
	for (double *mean = means; mean < meanMax; mean++)
		*mean /= numTests;
	

	// compute variances
	if (numTests > 1) {
		double temp;
		unsigned **durations = timeCompare.durations;
		for (double *mean = means, *var = vars; var < varMax; var++, mean++, durations++) {
			for (unsigned *duration = *durations, *durationMax = duration + numTests; duration < durationMax; duration++) {
				temp = *duration - *mean;
				*var += temp * temp;
			}
		}
		for (double *var = vars; var < varMax; var++)
			*var /= numTests - 1;
	}

	// compute compared efficiency metrics
	unsigned *solve1 = timeCompare.solves;
	for (double *eff = effs, *mean1 = means; mean1 < meanMax; mean1++, solve1++) {
		unsigned *solve2 = timeCompare.solves;
		for (double *mean2 = means; mean2 < meanMax; mean2++, solve2++, eff++)
			*eff = (*solve1 * *mean2) / (*solve2 * *mean1);
	}

	// prep stat display
	string *name = names;
	unsigned maxNameLength = 0; 
	unsigned nameLengths[numSolvers] = {}; // initialize to zero
	unsigned *nameLength = nameLengths;
	for (unsigned s = 0; s < numSolvers ; s++, name++, nameLength++) {
		*nameLength = name->length();
		if (*nameLength > maxNameLength) maxNameLength = *nameLength;
	}
	cout << "\n\n";

	// display stats
	unsigned statwidth = 12;
	unsigned numstats = 4;
	cout << "|" << setw(maxNameLength+1) << "STATS" << " |"
		 << setw(statwidth) << "Time Mean" << " |"
		 << setw(statwidth) << "Time Var" << " |"
		 << setw(statwidth) << "Solved" << " |"
		 << setw(statwidth) << "Solve Rate" << " |\n";
	cout << '+' << setw(maxNameLength+3) << setfill('=') << '+';
	for (unsigned j = 0; j < numstats; j++) {
		cout << setw(statwidth+2) << '+';
	}
	cout << setfill(' ') << '\n';
	double solveFactor = 100. / (numTests * numPuzzles);
	for (unsigned i = 0; i < numSolvers; i++) {
		cout << "|" << setw(maxNameLength+1) << names[i] << " |"
		 	 << setw(statwidth-1) << means[i] * 1e-3 << "s |"
		 	 << setw(statwidth-1) << vars[i] * 1e-6 << "s |"
		 	 << setw(statwidth) << timeCompare.solves[i] << " |"
			 << setw(statwidth-1) << timeCompare.solves[i] * solveFactor << "% |\n";
	}
	cout << '\n';

	// display comparison
	unsigned widths[numSolvers];
	cout << "|" << setw(maxNameLength+1) << "Puzzle/Puzzle" << " |";
	for (unsigned j = 0; j < numSolvers; j++) {
		unsigned w = (maxNameLength - nameLengths[j]) / 2 + 1;
		widths[j] = w*2 + nameLengths[j];
		cout << setw(w) << "";
		cout << names[j];
		cout << setw(w) << "";
		cout << "|";
	}
	cout << '\n' << '+' << setw(maxNameLength+3) << setfill('=') << '+';
	for (unsigned j = 0; j < numSolvers; j++) {
		cout << setw(widths[j]+1) << '+';
	}
	cout << setfill(' ') << '\n';
	double *eff = effs;
	for (unsigned i = 0; i < numSolvers; i++) {
		cout << "|" << setw(maxNameLength+1) << names[i] << " |";
		for (unsigned j = 0; j < numSolvers; j++, eff++) {
			cout << setw(widths[j]) << *eff << "|";
		}
		cout << '\n';
	}
}

void RUN_BENCHMARKS(int argc, char **argv) {
	DEBUG_FUNC_HEADER("RUN_BENCHMARKS(%d, char**)", argc)
	unsigned numPuzzles = 1, numTests = 1;
	if (argc > 1) numPuzzles = atoi(argv[1]); 
	if (argc > 2) numTests = atoi(argv[2]);

	time_compare_t timeCompare {NUM_SOLVERS, numTests};

	compareSolvers(numTests, numPuzzles, NUM_SOLVERS, SOLVER_NAMES, SOLVERS, timeCompare);
	displayComparisonStats(numTests, numPuzzles, NUM_SOLVERS, SOLVER_NAMES, timeCompare);

	cout << endl;
	DEBUG_FUNC_END()
}