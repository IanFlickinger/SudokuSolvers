#include <iostream>
#include <iomanip>
#include <chrono>

#include "puzzle.h"
#include "display.h"
#include "data.h"
#include "solvers.h"

#include "benchmarking.h"

using namespace std;

BenchmarkPuzzleLoader * BenchmarkPuzzleLoader::GetInstance() {
	static BenchmarkPuzzleLoader bpl;
	return &bpl;
}

SolverList * SolverList::GetInstance() {
    static SolverList sl;
    return &sl;
}

SolverList::SolverList(unsigned maxcap) : maxcap(maxcap), capinc(maxcap), curcap(0) {
    solvers = new Solvers::Solver*[maxcap];
    names = new std::string[maxcap];
}

SolverList::~SolverList() {
    for (unsigned i = 0; i < curcap; i++) delete solvers[i];
    delete[] solvers;
}

int SolverList::addSolver(Solvers::Solver *solver, std::string name) {
    if (curcap >= maxcap) {
        maxcap += capinc;
        Solvers::Solver **solversExt = new Solvers::Solver*[maxcap];
        std::string *namesExt = new std::string[maxcap];
        for (unsigned i = 0; i < curcap; i++) {
            solversExt[i] = solvers[i];
            delete solvers[i];
            namesExt[i] = names[i];
        }
        delete[] solvers;
        solvers = solversExt;
    }
    names[curcap] = name;
    solvers[curcap++] = solver;

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
	#ifdef DEBUG_MAIN_CPP
	 cout << "compareSolvers(Solver&, Solver&, " << numTests << ", " << numPuzzles  << ")" << endl;
	#endif
	#ifdef DEBUG_MAIN_CPP_VERBOSE
	 cout << setw(55) << setfill('=') << "" << endl;
	#endif
	Puzzle puzzles[numPuzzles]; 

	unsigned start, stop, duration, numSolved;
	for (unsigned testNum = 0; testNum < numTests; testNum++) {
		// Sample puzzles
		#ifdef DEBUG_MAIN_CPP_VERBOSE
		 cout << "Sampling " << numPuzzles << " Puzzles" << endl;
		#endif
		for (Puzzle *ptr = puzzles, *maxPtr = ptr+numPuzzles; ptr < maxPtr; ptr++) {
			Puzzle temp = SUDOKU_PUZZLE_LOADER.load();
			ptr->swap(temp);
		}
		#ifdef DEBUG_MAIN_CPP_VERBOSE
		if (numPuzzles == 1) Display::showPuzzle(*puzzles);
		#endif

		// Iterate over solvers
		for (unsigned solverNum = 0; solverNum < numSolvers; solverNum++) {
			Solvers::Solver &solver = *solvers[solverNum];
			std::string &name = names[solverNum];

			// Test solver 
			#ifdef DEBUG_MAIN_CPP_VERBOSE
			cout << "Running " << name << " solver" << endl;
			#endif
			start = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			for (Puzzle *puzzle = puzzles, *puzzleMax = puzzles + numPuzzles; puzzle < puzzleMax; puzzle++) {
				solver.solve(*puzzle);
			}
			stop = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			
			#ifdef DEBUG_MAIN_CPP_VERBOSE
			if (numPuzzles == 1) Display::showPuzzle(*puzzles);
			#endif

			// Track stats
			duration = stop - start;
			timeCompare.durations[solverNum][testNum] = duration;

			numSolved = 0;
			for (unsigned i = 0; i < numPuzzles; i++) numSolved += puzzles[i].isSolved();
			timeCompare.solves[solverNum] += numSolved;

			cout << name << " solver solved " << numSolved << " out of " << numPuzzles << " puzzles in " 
				<< setprecision(5) << duration * 1e-3 << " seconds\n";

			// Reset
			#ifdef DEBUG_MAIN_CPP_VERBOSE
			cout << "Resetting Puzzles" << endl;
			#endif
			for (unsigned i = 0; i < numPuzzles; i++) puzzles[i].reset();
		}
	}
}

void displayComparisonStats(
	unsigned numTests, 
	unsigned numPuzzles, 
	unsigned numSolvers, 
	std::string *names, 
	time_compare_t &timeCompare
) {
	// initialize means and variances
	double means[numSolvers], *meanMax = means + numSolvers;
	double vars[numSolvers], *varMax = vars + numSolvers;
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

	// display stats
	cout << "\n\n";
	std::string *name = names;
	unsigned maxNameLength = 0;
	unsigned nameLengths[numSolvers] = {}; // initialize to zero
	unsigned *nameLength = nameLengths;
	unsigned *solved = timeCompare.solves;
	for (double *mean = means, *var = vars; 
		 mean < meanMax; name++, mean++, var++, solved++, nameLength++
	) {
		cout << *name << " Execution Stats\n"
		 	 << "\tMean: " << *mean * 1e-3 << " seconds\n"
		 	 << "\tVariance: " << *var * 1e-6 << " seconds\n"
		 	 << "\tSolve Rate: " << *solved * 1e2 / (numTests * numPuzzles) << "%\n";
		*nameLength = name->length();
		if (*nameLength > maxNameLength) maxNameLength = *nameLength;
	}
	cout << endl;

	// display comparison
	unsigned widths[numSolvers];
	cout << "|" << setw(maxNameLength+1) << "" << " |";
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
	for (unsigned i = 0; i < numSolvers; i++) {
		cout << "|" << setw(maxNameLength+1) << names[i] << " |";
		for (unsigned j = 0; j < numSolvers; j++) {
			cout << setw(widths[j]) << "" << "|";
		}
		cout << '\n';
	}
		//  << "Solver " << (meanA < meanB ? 'A' : 'B') << " is faster by factor "
		//  << (meanA > meanB ? meanA / meanB : meanB / meanA) << "s/s"
		//  << endl;
}

void RUN_BENCHMARKS(int argc, char **argv) {
	unsigned numPuzzles = 1, numTests = 1;
	if (argc > 1) numPuzzles = atoi(argv[1]); 
	if (argc > 2) numTests = atoi(argv[2]);

	time_compare_t timeCompare {NUM_SOLVERS, numTests};

	compareSolvers(numTests, numPuzzles, NUM_SOLVERS, SOLVER_NAMES, SOLVERS, timeCompare);
	displayComparisonStats(numTests, numPuzzles, NUM_SOLVERS, SOLVER_NAMES, timeCompare);

	cout << endl;
}