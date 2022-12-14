#ifndef SUDOKU_GRAPH_H
#define SUDOKU_GRAPH_H

unsigned *** graphNeighborhoodByCell(unsigned size);
unsigned *** graphNeighborhoods(unsigned size);
unsigned ** graphNeighborsByCell(unsigned size);

#endif