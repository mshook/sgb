# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About This Project

This is the **Stanford GraphBase (SGB)** by Donald E. Knuth — a collection of datasets and programs for generating and examining graphs and networks, originally published in 1993.

The repository's **master** branch holds all official DEK releases (1992–2025). The **local** branch contains further improvements: compiler-warning fixes via changefiles in `PROTOTYPES/`, a `sgb.spec` for building rpm/deb packages, and extraction of a shared object `libgb.so`.

## Source Code Format: CWEB (Literate Programming)

All source files use the [CWEB](https://www-cs-faculty.stanford.edu/~knuth/cweb.html) literate programming system. This is critical to understand:

- **`.w` files** — CWEB source (interleaved documentation and C code)
- **`.ch` files** — Change files that patch `.w` files for specific platforms or purposes
- **`ctangle foo.w`** — Extracts C code → `foo.c`
- **`cweave foo.w`** — Extracts TeX documentation → `foo.tex`
- If a `.ch` file exists: `ctangle foo.w foo.ch` applies the patch before extraction

The Makefile's `.w.c` rule handles this automatically: `if test -r $*.ch; then ctangle $*.w $*.ch; else ctangle $*.w; fi`

**Do not edit `.c` or `.h` files directly** — they are generated from `.w` sources.

## Prerequisites

CWEB version 3.0 or greater must be installed (`ctangle` and `cweave` commands must be available).

## Build Commands

```sh
make tests        # Build and run all tests; creates 'certified' on success
make install      # Install library and headers to $SGBDIR (requires 'certified')
make installdemos # Build and install all demonstration programs
make clean        # Remove generated .c, .h, .o, .tex, .dvi, libgb.a, certified
make veryclean    # clean + remove demo binaries
make lib          # Build libgb.a only
```

Before running `make tests`, edit the `Makefile` to set `SGBDIR`, `DATADIR`, `INCLUDEDIR`, `LIBDIR`, `BINDIR`, and `CWEBINPUTS` — or uncomment the SHORTCUT lines for single-directory installation.

## Architecture

### Module Layers

**Kernel** (must be built first, form the core of `libgb.a`):
- `gb_graph.w` — Core data structures: `Graph`, `Vertex`, `Arc`; memory allocation via `Area`
- `gb_io.w` — I/O routines for reading `.dat` files
- `gb_flip.w` — System-independent random number generator
- `gb_sort.w` — Linked-list sort

**Graph Generators** (also in `libgb.a`): `gb_basic`, `gb_books`, `gb_econ`, `gb_games`, `gb_gates`, `gb_lisa`, `gb_miles`, `gb_plane`, `gb_raman`, `gb_rand`, `gb_roget`, `gb_words` — each reads from a corresponding `.dat` file.

**Support** (also in `libgb.a`): `gb_dijk.w` (Dijkstra's algorithm variants), `gb_save.w` (graph serialization).

**Demonstrations**: `assign_lisa`, `book_components`, `econ_order`, `football`, `girth`, `ladders`, `miles_span`, `multiply`, `queen`, `roget_components`, `take_risc`, `word_components` — standalone programs linking against `libgb.a`.

### Changefiles by Purpose

| Directory    | Purpose |
|-------------|---------|
| `ANSI/`     | ANSI C compliance patches (original, minimal) |
| `PROTOTYPES/` | Full ANSI prototype patches + AMIGA support; fixes `gcc -Wall -Wextra` warnings |
| `AMIGA/`    | Amiga-specific build files |
| `MSVC/`     | Visual Studio project files |

To use `PROTOTYPES/` patches on Unix: copy `.ch` files to the root directory — the Makefile automatically detects and applies them via the `.w.c` rule.

### Known Non-Standard C

The code intentionally uses non-ANSI pointer arithmetic (comparing/ordering pointers of unrelated origin). This is noted in the README and is by design. The `PROTOTYPES/` changefiles add function prototypes but do not fix the pointer issues.

## GB_GATES and the RISC Machine

`gb_gates.w` is one of the more analytically interesting modules. It produces directed acyclic graphs of logic gates representing two circuits: a parallel multiplier (`prod`) and a simple RISC processor (`risc`). The demo programs `TAKE_RISC` and `MULTIPLY` show how to use them.

The `risc(regs)` function (2 ≤ regs ≤ 16) builds a gate-level netlist of a 16-bit processor with `1400 + 115*regs` gates. Key architectural facts established by reading the source:

- **r0 is the program counter** — same design as DEC PDP-11 (r7=PC) and early ARM (r15=PC); control flow is just a register write; JUMP with DST≠0 saves a return address (like ARM `BL`)
- **Read-only memory** — the machine cannot write to memory; this keeps the circuit a DAG (no feedback loops), making it theoretically a finite automaton rather than a von Neumann machine
- **OP=0 general logical** — a 4-bit truth-table MOD field subsumes all two-input boolean ops in one instruction; the closest modern equivalent is Intel AVX-512 `VPTERNLOGD`
- **No XOR gates** in the netlist — XOR is synthesized from AND/OR/NOT
- Arc weights of 100 (gates) vs 1 (inverters) make shortest-path lengths in the graph meaningful as gate-delay estimates

Full ISA reference: `RISC_DESCRIPTION.md`. Block diagram: `risc_architecture.svg`.

## Session Notes

Dated session summaries (conversation logs) are saved as `session-YYYY-MM-DD.md`.
