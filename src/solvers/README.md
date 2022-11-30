# Solvers

This directory contains all of the sudoku-solving algorithms separated by approach. This README contains:
 - [Algorithm Overviews](#algorithms)
 - [Theoretical Comparisons](#algorithm-comparison)
 - [Empirical Comparisons](#tests)

## Algorithms
----------------
## Basic Searches

**Depth-First Search**

Depth-First Search is based on the premise of treating the sudoku state space as a searchable tree graph. Here, the state space is the space of all possible configurations of values in the grid - whether or not they are valid sudoku solutions. 

We define the state space graph as such:

    A sudoku state *S* is the child of another sudoku state *T* if and only if *S* can be obtained from *T* by selecting an empty cell of *T* and assigning a value to it. Again - this value does not have to be correct.

For example, the following image shows two incomplete 4x4 sudokus. The sudoku on the right is obtained by setting the upperleftmost value to 3. Thus, in the 4x4 sudoku state space, the sudoku state on the right is a child of the state on the left.

### Simulated Annealing

| Schedule | Equation |
| --- | --- |
| Geometric | t = a * t |

### Collapsing Graph

To build a thorough understanding of the collapsing graph solution to the Sudoku puzzle, we will begin by defining the Sudoku as a graph. While this is relatively straightforward for those familiar with Computer Science concepts and/or mathematics, it is important to stress that the definition of the graph here

We use the barycentric coordinate system. Why? Becuase I have a hunch that plain coordinates just aren't good enough.

### Evolutionary Approaches

### Population-Based Approaches

## Algorithm Comparison
-----------------