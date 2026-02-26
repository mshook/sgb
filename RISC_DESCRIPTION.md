# The RISC Machine (`risc` routine, `gb_gates.w`)

## What `risc` Produces

`risc(regs)` builds a **directed acyclic graph of logic gates** — not a simulation, but an actual gate-level netlist of a small microprocessor. `regs` must be between 2 and 16 (defaults to 16 otherwise). The resulting graph has `1400 + 115 * regs` gates (1,630 at minimum, 3,240 at maximum). No XOR gates are used; XOR effects are achieved purely through AND, OR, and NOT gates.

---

## Inputs and Outputs

The circuit has **17 inputs** and **16 outputs**:

| Signal | Direction | Description |
|---|---|---|
| `RUN` | Input (bit 0) | If 0, all registers and outputs are cleared to 0 |
| Memory data | Input (bits 1–16) | 16-bit contents of the previously-addressed memory location, **little-endian** (LSB first) |
| Memory address | Output (bits 0–15) | 16-bit address register, **big-endian** (MSB first) |

The machine is **read-only**: it can read from external memory but never write to it. To run the machine you feed the output address back into the input on each cycle, accessing real memory in between calls to `gate_eval`.

---

## Registers

The machine has **2–16 general-purpose 16-bit registers** (r0–r15). All data words are 16 bits wide.

**Register 0 is special — it is the program counter (PC).** If an instruction does not modify r0, it automatically increments by 1. Writing to r0 redirects control flow.

Registers not physically instantiated (when `regs < 16`) always read as zero, but writes to them still affect status bits.

---

## Four Status Bits

| Bit | Name | Meaning |
|---|---|---|
| `S` | Sign | Bit 15 (MSB) of the most recent result |
| `N` | Nonzero | 1 if any of bits 0–14 of the result are 1 |
| `K` | Carry/Borrow | Carry out of addition, or borrow from subtraction |
| `V` | Overflow | Signed arithmetic overflow |

`S` and `N` are set by logical and shift/arithmetic operations. `K` and `V` are set only by arithmetic and shift operations.

---

## Instruction Format (16 bits)

```
 15  14  13  12 | 11  10   9   8 |  7   6 |  5   4 |  3   2   1   0
[    DST (4b)   ] [   MOD (4b)   ] [OP 2b] [A  2b ] [   SRC (4b)   ]
```

- **DST** — destination register (0–15)
- **MOD** — modifier; meaning depends on OP
- **OP** — operation code (2 bits → 4 operations)
- **A** — addressing mode
- **SRC** — source specifier

---

## Addressing Modes (`A` field)

| A | Source value is... |
|---|---|
| 0 | SRC interpreted as a **4-bit signed immediate** (−8 to +7) |
| 1 | Contents of **register DST + signed(SRC)** |
| 2 | Contents of **register SRC** |
| 3 | Contents of **memory[register SRC]** |

**Special two-word instruction:** When `A=3` and `SRC=0`, the source would normally be `memory[r0]` — but that is the current instruction itself. Instead, the machine uses the *following* memory location as a 16-bit immediate operand, and r0 increments by 2 instead of 1.

---

## Operations (`OP` field)

### `OP=0` — General Logical Operation

MOD's four bits `μ11 μ10 μ01 μ00` define a truth table. For each bit position, if the current destination bit is `i` and the source bit is `j`, the result is `μ_ij`. Examples:

| MOD (binary) | Effect |
|---|---|
| `1010` | Copy source → destination |
| `0110` | XOR |
| `0011` | Complement destination (source ignored) |
| `0000` | Clear destination |

Sets `S` and `N`; does **not** affect `K` or `V`.

---

### `OP=1` — Arithmetic and Shift Operations (selected by MOD)

| MOD | Operation |
|---|---|
| 0 | Shift left 1 (×2) |
| 1 | Cyclic shift left 1 (×2, MSB wraps to LSB) |
| 2 | Shift left 4 (×16) |
| 3 | Cyclic shift left 4 |
| 4 | Arithmetic shift right 1 (÷2, rounds toward −∞) |
| 5 | Unsigned shift right 1 (MSB set to 0) |
| 6 | Arithmetic shift right 4 (÷16) |
| 7 | Unsigned shift right 4 |
| 8 | Add source to destination |
| 9 | Add source + K to destination (for 32-bit chained addition) |
| 10 | Subtract source from destination |
| 11 | Subtract source + K from destination (for 32-bit chained subtraction) |
| 12–14 | Reserved — set destination and all status bits to **0** |
| 15 | **JUMP** (see below) |

All shift and arithmetic operations set `S`, `N`, `K`, and `V`.

**JUMP (`OP=1`, `MOD=15`):** Moves source into r0 (redirecting the PC). If `DST ≠ 0`, saves the address of the instruction after the JUMP into register DST — the standard way to call a subroutine. JUMP does **not** affect `S`, `N`, `K`, or `V`.

---

### `OP=2` — Conditional Load on S and N

Loads source into DST only if `μ_{S,N} = 1` in MOD. Does **not** change `S` or `N`. Example: `MOD=1111` is an unconditional move; `MOD=0011` loads if `S=0` (result ≥ 0).

### `OP=3` — Conditional Load on K and V

Same mechanism but tests `K` and `V`. Does not change `K` or `V`. Example: `MOD=1010`, `OP=3`, `A=1`, `SRC=1` adds the current carry/overflow bit to the destination.

---

## Simulation via `run_risc`

`run_risc(g, rom, size, trace_regs)` simulates the machine starting at address 0, halting when the PC exceeds `size`. Final register contents are stored in the global array `risc_state[]`. A common halt idiom is the instruction `0x0F00`, which jumps to address `0xFFFF`.
