#ifndef SUDOKU_DISPLAY_H
#define SUDOKU_DISPLAY_H

#include "puzzle.h"

#define DISPLAY_HORIZONTAL_SEPARATOR "+---------+---------+---------+"

namespace Display {

void showPuzzle(const Puzzle&);
void clear();

}

#endif