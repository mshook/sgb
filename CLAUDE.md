# Stanford GraphBase

A collection of graph datasets and algorithms by Donald E. Knuth (1993), written in CWEB literate programming style.

## Build

```bash
make tests        # build and run test programs
make lib          # build libgb.a
make demos        # build demonstration programs
make all          # build everything
```

Requires `ctangle`/`cweave` (CWEB tools) and a C compiler.

## Source Files

- `*.w` — CWEB source files (literate C); run `ctangle foo.w` to produce `foo.c`
- `*.dat` — graph datasets (anna, david, econ, games, homer, huck, jean, lisa, miles, roget, words)
- `*.ch` — CWEB change files for platform variants (see `ANSI/`, `PROTOTYPES/`, `AMIGA/`, `MSVC/`)

## Core Library (`libgb.a`)

| Module | Purpose |
|---|---|
| `gb_graph.w` | Graph data structures |
| `gb_io.w` | I/O routines |
| `gb_flip.w` | Portable random number generator |
| `gb_sort.w` | Linked-list sorting |

## Platform Notes

- Primary target: Unix/Linux
- `ANSI/` — ANSI C compatibility changefiles
- `MSVC/` — Microsoft Visual C++ support
- `AMIGA/` — Commodore Amiga support
