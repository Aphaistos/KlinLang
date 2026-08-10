# Klin

**Klin** (or simply **Klin**) is a low-level, imperative, and predictable systems programming language designed to provide total control over hardware and memory. It operates without runtime overhead, garbage collection, or verbose syntax.

Klin compiles directly into **FASM (Flat Assembler)** assembly, producing compact, fast binaries with clear, 1-to-1 machine-instruction predictability.

---

## Rationale & Core Concepts

The name **Klin** (pronounced like *"clean"*) reflects the core design goal of the language: providing a **clean, transparent, and unpolluted** programming environment free of hidden runtime mechanics, implicit heap allocations, or syntactic noise.

1. **The Bubble Metaphor (`::`)**: A unified geometric model that harmonizes namespaces, visibility, and memory allocation scopes into a single syntax.
2. **Arena-Based Memory**: No hidden allocations, no scattered `malloc`/`free`. Memory is managed explicitly, locally, and deterministically through arena allocators.
3. **Bare-Metal & Systems Control**: Native support for packed structures (`[]`), forced alignments (`[N]`), and bit-fields directly within struct definitions.
4. **Clean, Uncluttered Syntax**: Minimal syntactic noise—no explicit `self` pointers, clear return arrows (`->`), and lightweight explicit annotations.

---

## Project Structure & Manifest

In Klin, source files are neutral containers. The global architecture of a module is explicitly defined by a **manifest file** formatted as a dependency graph.

### Example Directory Tree

```text
./
├── main.kln
├── std.kln       // Manifest for the std module
└── std/
    ├── io.kln    // Contains ::io { ... }
    └── net.kln   // Contains ::net { ... }

```

### The Manifest (`std.kln`)

The `->` arrow indicates the paths to the files constituting the module universe:

```klin
mod ::std {
    -> "std/io.kln"
    -> "std/net.kln"
}

```

### Compilation Command

```bash
klin main.kln std.kln

```

---

## Language Overview

### 1. Universes & Gateways (`imp` and `use`)

```klin
// --- main.kln ---
imp std::io;   // Establishes a path to the 'io' bubble in the 'std' universe
use std::io;   // Brings 'io::' directly into the local scope without parent prefixes

func main() -> i32 {
    io::println("Hello World!");
    -> 0; // Explicit return arrow
}

```

### 2. Memory Layouts & Bare-Metal Features

Klin allows memory layout adjustments without verbose attributes:

```klin
// Structure aligned on a 16-byte boundary (e.g., SIMD / AVX)
[16] struct Vector4 {
    x: f32,
    y: f32,
    z: f32,
    w: f32
}

// Packed structure without padding, including native bit-fields
[] struct NetworkHeader {
    version: u8 : 4,    // 4-bit bit-field
    ihl: u8 : 4,        // 4-bit bit-field
    tos: u8,
    len: u16
}

```

### 3. Allocation Bubbles (`::(arena)`)

Arenas provide fast, fragmentation-free memory allocation:

```klin
imp std::mem;

func process_frame(scratch: *mem::Arena) {
    // Allocation bubble linked to the 'scratch' arena
    ::(scratch) {
        val temp_buffer = alloc[u8; 2048];
        val header = alloc[NetworkHeader; 1];

        // Frame processing...
    } 
    // Upon exiting the bubble, the arena offset is 
    // automatically restored / reset in O(1) time.
}

```

---

## Summary of Scope Rules (`::`)

| Syntax | Description | Purpose |
| --- | --- | --- |
| `mod ::std` | **Module / Universe** | Declares the root of a package or module. |
| `::io` | **Public Space** | Symbol organization namespace. |
| `::_win32` | **Private Space** | Sealed bubble inaccessible from external modules. |
| `::(arena)` | **Memory Bubble** | Allocation scope bound to a specific arena. |
| `{ ... }` | **Stack Bubble** | Standard stack frame for local variables. |

---

## Roadmap & Status

* [x] Grammar and syntax design
* [x] Namespace modeling and manifest specification
* [x] Arena memory model & bare-metal layout specification
* [ ] Parser & AST implementation
* [ ] FASM code generator (x86-64)

---

> *Project currently in design and early development phase.*
