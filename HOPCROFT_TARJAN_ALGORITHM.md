# Hopcroft-Tarjan Biconnectivity Algorithm

## Overview

The Hopcroft-Tarjan biconnectivity algorithm, as implemented in `book_components.w`, finds **biconnected components** and **articulation points** of a graph in O(V+E) time using depth-first search.

## Key Definitions

- **Biconnected component (bicomponent):** A maximal set of edges where any two edges lie on a common simple cycle. Equivalently, a connected subgraph where removing any single vertex leaves it connected.
- **Articulation point:** A vertex shared by two or more bicomponents — removing it disconnects the graph.

## Data Structures

Each vertex maintains 5 fields:

| Field | Purpose |
|---|---|
| `rank` | DFS discovery order (0 = unseen) |
| `parent` | Parent in the DFS tree |
| `untagged` | Pointer to next unexplored arc |
| `link` | Stack pointer for `active_stack` |
| `min` | Lowest-rank ancestor reachable via tree arcs + one back edge |

The `min` field is the critical insight. For vertex `v`, it points to the active vertex of **smallest rank** reachable by following zero or more mature tree arcs downward from `v`, then one non-tree (back) edge upward.

## Algorithm Walkthrough

### 1. Initialization

All vertices start unseen (`rank=0`), all arcs untagged. A `dummy` vertex with rank 0 serves as the virtual parent of each DFS root.

### 2. DFS Exploration

At each step, from current vertex `v`:

- **If `v` has an untagged arc** to vertex `u`:
  - If `u` is already seen (has rank): this is a **back edge**. Update `v->min` if `u->rank < v->min->rank`.
  - If `u` is unseen: this is a **tree edge**. Set `u->parent = v`, push `u` onto the active stack, make `u` the new current vertex.

- **If all arcs from `v` are tagged** (v matures), let `u = v->parent`:
  - If `v->min == u`: vertex `v` and all its descendants on the active stack form a **bicomponent** together with `u`. Pop them off and report them.
  - Otherwise: propagate — if `v->min->rank < u->min->rank`, update `u->min = v->min`. Then backtrack to `u`.

### 3. Bicomponent Detection

When `v->min == v->parent`, it means there is **no back edge** from `v` or any descendant that reaches above `v`'s parent `u`. Therefore `u` is the only way to reach `v`'s subtree from the rest of the graph — making `u` an articulation point and `{v, descendants of v, u}` a bicomponent.

## The Core Invariant

Knuth's "cave analogy": think of vertices as rooms and edges as passageways. When you enter room `v` from room `u`, if there's no way out of the subcave starting at `v` without passing back through `u`, and every descendant of `v` can reach `u` without going through `v`, then `v`'s subcave plus `u` form a bicomponent.

## Correctness Proof (Summary)

Knuth proves three key properties:

1. The cycle-equivalence relation on edges is transitive (hence a valid equivalence relation).
2. A graph is biconnected **iff** for any three distinct vertices x, y, z, there exists a path from x to y avoiding z (property P).
3. The subgraph extracted when `v->min == v->parent` satisfies property P, because every non-root vertex `x` has a path to its grandparent avoiding its parent (via `x->min`).

## Example

```
Graph:  A--B--D--E
        |×|  |
        C    F

DFS from A: ranks A=1, C=2, D=3, B=4, E=5, F=6
Back edge B→A gives min(B)=A, propagated up: min(D)=A, min(C)=A

When E matures: min(E)=D=parent → bicomponent {D,E,F}  (D is articulation point)
When C matures: min(C)=A=dummy → bicomponent {A,B,C,D}
```

## Time Complexity

The algorithm processes each edge exactly once, giving **O(V+E)** time complexity.

## References

- Implementation: `book_components.w` (Stanford GraphBase)
- Hopcroft, J. E., & Tarjan, R. E. (1973). "Algorithm 447: Efficient algorithms for graph manipulation." Communications of the ACM, 16(6), 372-378.
