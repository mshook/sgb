# The `.gb` File Format

The `.gb` format is the plain-text, human-and-machine-readable graph serialization
format used by the Stanford GraphBase. It is defined in `gb_save.w`, which provides
two routines: `save_graph(g, "foo.gb")` and `restore_graph("foo.gb")`.

## Overall Structure

```
* GraphBase graph (util_types XXXXXXXXXXXXXXXX,NV,MA)
"graph_id",n,m[,utility_fields...]
* Vertices
[one line per vertex]
* Arcs
[one line per arc]
* Checksum 271828
```

## Line 1 — Header

```
* GraphBase graph (util_types IZAZZZZVZZZZSZ,3V,4A)
```

- Must start with `* GraphBase graph`
- `util_types` — exactly **14 characters** from the set `{Z, I, V, S, A}`, encoding
  the types of utility fields:
  - `Z` — ignored (zero/null)
  - `I` — integer (`long`)
  - `V` — vertex pointer (`Vertex*`)
  - `S` — string pointer (`char*`)
  - `A` — arc pointer (`Arc*`)
- The 14 positions map to: `u v w x y z` of vertices (positions 0–5), `a b` of arcs
  (positions 6–7), `uu vv ww xx yy zz` of the graph record (positions 8–13)
- `NV` — total number of `Vertex` records
- `MA` — total number of `Arc` records

Lines starting with `*` that appear before this header line are treated as free-form
comments and are not included in the checksum calculation.

## Line 2 — Graph Record

```
"id",n,m[,utility_fields...]
```

- `"id"` — the graph's identity string (quoted; see String Encoding below)
- `n` — number of vertices (integer)
- `m` — number of arcs (integer)
- Followed by the 6 graph-level utility fields (`uu`–`zz`), in order; fields whose
  `util_types` character is `Z` are omitted entirely

Fields within a record are comma-separated. A trailing comma means the record
continues on the next line.

## `* Vertices` Section

One line per vertex, in index order (V0, V1, …):

```
"name",arc_ptr[,utility_fields...]
```

- `"name"` — vertex name string (quoted)
- `arc_ptr` — head of the adjacency list: `A`*k* (arc index) or `0` (NULL)
- Followed by the 6 vertex utility fields (`u`–`z`), only non-`Z` ones

## `* Arcs` Section

One line per arc, in index order (A0, A1, …):

```
tip_ptr,next_ptr,len[,utility_fields...]
```

- `tip_ptr` — destination vertex: `V`*k* (vertex index), `0` (NULL), or `1` (special
  sentinel used by `gb_gates` for the complemented-arc convention)
- `next_ptr` — next arc in the linked list: `A`*k* or `0` (NULL)
- `len` — arc length (signed integer)
- Followed by the 2 arc utility fields (`a`, `b`), only non-`Z` ones

## `* Checksum` Line

```
* Checksum 271828
```

A single integer computed incrementally over every checksummed line in the file
(everything after the header). A negative value disables the check. **Do not
hand-edit `.gb` files**, as an incorrect checksum causes `restore_graph` to fail
with `late_data_fault`.

## Pointer Encoding

| Syntax  | Meaning                                            |
|---------|----------------------------------------------------|
| `V`*k*  | vertex at zero-based index *k*                     |
| `A`*k*  | arc at zero-based index *k*                        |
| `0`     | NULL pointer                                       |
| `1`     | special vertex sentinel (`gb_gates` only)          |

## String Encoding

- Delimited by double-quote characters: `"..."`.
- The characters `\`, `"`, and newline may not appear inside a string; `save_graph`
  replaces illegal characters with `?` and sets the `bad_string_char` anomaly flag.
- A string that would exceed 79 characters on one line is broken across lines by
  writing `\` as the last character before the newline; `restore_graph` splices the
  parts back together automatically.
- Maximum string length: 4095 characters. Maximum `id` string length: 154 characters.

## Worked Example (from `gb_save.w`)

```
* GraphBase graph (util_types IZAZZZZVZZZZSZ,3V,4A)
"somewhat_contrived_example(3.14159265358979323846264338327\
9502884197169399375105820974944592307816406286208998628)",1,3,"pi"
* Vertices
"look",A0,15,A1
"feel",0,-9,A1
"",0,0,0
* Arcs
V0,A2,3,V1
V1,0,5,0
V1,0,-8,1
0,0,0,0
* Checksum 271828
```

Here `util_types` is `IZAZZZZVZZZZSZ`:
- Vertex fields: `u.I`, `v.Z`, `w.A`, `x.Z`, `y.Z`, `z.Z` — so each vertex line has
  `name`, `arcs`, `u` (integer), and `w` (arc pointer)
- Arc fields: `a.Z`, `b.V` — so each arc line has `tip`, `next`, `len`, and `b`
  (vertex pointer)
- Graph fields: `uu.Z vv.Z ww.Z xx.Z yy.S zz.Z` — so the graph line has `id`, `n`,
  `m`, and `yy` (string, `"pi"`)

## Constraints

- Only data reachable from `g->data` is saved; `g->aux_data` is silently discarded.
- Each memory block in `g->data` must be "pure": it may hold only `Vertex` records,
  only `Arc` records, or only string characters — never a mixture.
- The `G` (graph-within-graph) utility type is not supported.
- Vertex and arc records retain their relative order from the original graph where
  possible; the relative order of string data in memory is not preserved.
