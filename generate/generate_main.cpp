#include <stdlib.h>
#include <iostream>
#include <string.h>

#include "puzzle.h"

using namespace std;

int main(int argc, char **argv) {
    // -o, --output filepath
    // -n, --number dataset_size
    // -s, --size puzzle_size
    // -pr, --resample-prob resampleP
    // -pa, --alter-prob alterP
    // -pg, --generate-prob generateP
    string filepath;
    unsigned datasetSize = 1;
    unsigned char puzzleSize = 9;
    double resampleP = -1, alterP = -1, generateP = -1;

    // read command flags
    for (unsigned arg = 1; arg < argc; arg++) {
        if (argv[arg][0] == '-') {
            if (!(strcmp(argv[arg], "-o") && strcmp(argv[arg], "--output")))
                filepath = string(argv[++arg]);
            else if (!(strcmp(argv[arg], "-n") && strcmp(argv[arg], "--number")))
                datasetSize = atoi(argv[++arg]);
            else if (!(strcmp(argv[arg], "-s") && strcmp(argv[arg], "--size")))
                puzzleSize = atoi(argv[++arg]);
            else if (!(strcmp(argv[arg], "-pr") && strcmp(argv[arg], "--resample-prob")))
                resampleP = atof(argv[++arg]);
            else if (!(strcmp(argv[arg], "-pa") && strcmp(argv[arg], "--alter-prob")))
                alterP = atof(argv[++arg]);
            else if (!(strcmp(argv[arg], "-pg") && strcmp(argv[arg], "--generate-prob")))   
                generateP = atof(argv[++arg]);
            else {
                cout << "Unknown flag: " << string(argv[arg]) << endl;
                return 1;
            }
        }
    }

    // process flag settings
    if (resampleP < 0 && alterP < 0 && generateP < 0)
        resampleP = alterP = generateP = 1 / 3.;
    else {
        double probsum = (resampleP < 0 ? 0 : resampleP) + (alterP < 0 ? 0 : alterP) + (generateP < 0 ? 0 : generateP);
        unsigned char empties = (resampleP < 0) + (alterP < 0) + (generateP < 0);
        if (probsum > 1 && empties > 1) {
            cout << "When supplying unnormalized probabilities, all probabilities must be specified. Received: ";
            if (resampleP > 0) cout << "\n\tResample Probability=" << resampleP;
            if (alterP > 0) cout << "\n\tAlter Probability=" << alterP;
            if (generateP > 0) cout << "\n\tGenerate Probabiltiy=" << generateP;
            return 1;
        }
        double initVal = (1 - probsum) / empties;
        if (resampleP < 0) resampleP = initVal;
        if (alterP < 0) alterP = initVal;
        if (generateP < 0) generateP = initVal;
    } 
    if (filepath.length() == 0) {
        string size = to_string(puzzleSize);
        filepath = size + "x" + size + ".csv";
    }

    Puzzle puzzle(puzzleSize);
    cout << "to_string(Puzzle)=" << puzzle.to_string() << endl;
}