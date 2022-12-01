# Solvers

This directory contains all of the sudoku-solving algorithms implemented in the library. In this README are descriptions and comparisons of the algorithms.

# Algorithms
## Basic Searches

**Depth-First Search**

Depth-First Search is based on the premise of treating the sudoku state space as a searchable tree graph. For more information on searching tree graphs and depth-first search, [this hackerearth article](https://www.hackerearth.com/practice/algorithms/graphs/depth-first-search/tutorial/) is a good starting point.

Here, the state space is the space of all possible configurations of values in the grid - whether or not they are valid sudoku solutions. We define the state space graph as such:

> A sudoku state *S* is the child of another sudoku state *T* if and only if *S* can be obtained from *T* by selecting an empty cell of *T* and assigning a value to it. This value does not have to be correct.

For example, the following image shows two incomplete 4x4 sudokus. The sudoku on the right is obtained by setting the upperleftmost value to 3. Thus, in the 4x4 sudoku state space, the sudoku state on the right is a child of the state on the left - despite the eggregious error.

![alt text](../../readme-images/dfs-example.jpg)

After a child state is selected (i.e., a guess is applied to the sudoku) it is evaluated to see if there are any constraint violations. If so, all children of this state will also contain that violation. Thus, the search can now prune the entire branch of sudoku states, backtrack to the last valid state, and continue the search. This process continues until a sudoku state in which all values are filled in with no constraint violations is found.

**Depth-First Search V1**

After implementing this algorithm, it was rewritten with several optimizations including pointer iteration, and rearranged logic. The original implementation was left in the code to be used for benchmarking other algorithms.

## Simulated Annealing

This algorithm was implemented according to [[1]](#references). 

Simulated annealing is a [metaheuristic](https://en.wikipedia.org/wiki/Metaheuristic "Wikipedia") which borrows from metallurgy concepts. For a great visual introduction to the idea of annealing, check out [Steve Mould's YouTube video](https://www.youtube.com/watch?v=xuL2yT-B2TM) *(8min)*.

In the video, Steve Mould demonstrates annealing using metallic balls which are settling due to gravity. The implementation of simulated annealing in the context of Sudoku is more analagous to a situation in which only one of those balls is animated while the rest are static, and the animated ball is searching for the lowest dimple in the terrain.

To implement the algorithm, a cost function has to be defined which acts as a measurement of the "height" of the ball. The optimal resting place for the ball is the location in which it has the lowest height.

In the Sudoku context, a good measurement of "height" is the number of constraint violations (i.e., the number of cells which share the same value as a neighbor).

After defining a cost (height) function. A temperature schedule needs to be defined. The higher the temperature, the more kinetic energy the ball has, and the more likely it is to bounce out of a local minimum. The lower the temperature, the less kinetic energy the ball has, and the less likely the ball is to bounce out of a local minimum. By progressively cooling the temperature, the ball is more likely to find itself in the global minimum of the terrain.

A number of temperature schedules were defined in a 1987 book titled *Simulated Annealing: Theory and Applications* [[2]](#references) for simulated annealing. A few are shown below where `t` represents the temperature.

| Schedule | Initialize | Update |
| --- | --- | --- |
| Geometric(<code>t<sub>0</sub></code>, `a`) | <code>t = t<sub>0</sub></code>| `t = a * t` |

Finally, a function `P(t, Δ)` which describes the acceptance probability (a.k.a. transition probability) must be defined. This function `P` depends on the temperature `t` and the change in height `Δ`. Note the dependence on the *change* in height as opposed to being solely dependent on the height. This allows the simulated annealing algorithm to make decisions locally - which is especially useful in contexts where the value of the global minimum is not known. The popularity vote for acceptance probability functions - and the choice for this algorithm - goes to the exponential function:

    P(t, Δ) = exp(Δ / t)

It is also often recommended to sample a series of random state changes (markov chain) in between temperature updates. This is the approach taken in [[1]](#references), recommended in [[2]](#references) and implemented here. As recommended in [[1]] the length of this chain is equal to the square of the number of empty cells in the provided puzzle.

## Collapsing Graph

In this approach, the Sudoku graph is not the state space graph as defined above in [depth first search](#basic-searches). 

## Evolutionary Approaches

## Population-Based Approaches

## Algorithm Comparison
-----------------

## References

**[1]** Rhyd Lewis. “Metaheuristics can solve sudoku puzzles”. In: Journal of Heuristics 13.4 (2007), pp. 387–401. doi: [10.1007/s10732-007-9012-8](doi.org/10.1007/s10732-007-9012-8).

**[2]** Laarhoven, et.al. "Simulated Annealing: Theory and Applications", 1987, Volume 37 ISBN : 978-90-481-8438-5.