# Virtual Machine Translator

This is a **Virtual Machine (VM) Translator** implemented in **C**, serving as a critical component of a two-tier compilation process. It translates high-level **VM code** (an intermediate language) into **assembly language** for a target machine, bridging the gap between high-level logic and low-level execution.

## Purpose

The VM Translator converts **intermediate VM code** into **assembly language** for the target machine. The output is then passed to an **assembler**, which produces the final **binary machine code**.

## Target Platform

This implementation targets the **Hack computer**, and all VM commands are translated into **Hack assembly**.

## Design

- Written in **C**
- Follows **object-oriented design principles**
- Mimics OOP concepts using C constructs

## Why C?

While C is not inherently object-oriented, it offers enough low-level control to emulate object-oriented behavior.
Using C posed an exciting challenge and aligned with the goal of building the full compilation pipeline from the ground up.

## VM Architecture: Stack-Based Design

The Virtual Machine operates on a **stack-based architecture**, perfectly aligned with the design philosophy of the **Hack platform**, a streamlined 16-bit system built for simplicity, featuring a compact memory model and optimized for efficient, fundamental instructions.

### Core Principles

- **Stack-Centric Execution**: All computations occur on a central stack using **push** and **pop** operations. The stack resides in the Hack RAM, typically starting at address `256` (SP).
- **No General-Purpose Registers**: Unlike register-based machines, this architecture relies solely on the stack and predefined memory segments.
- **Segmented Memory Model**: Memory is divided into logical segments that abstract different roles in program execution.

### Memory Segments (and Hack RAM Mapping)

| Segment    | Purpose                          | Hack Mapping                  |
| ---------- | -------------------------------- | ----------------------------- |
| `local`    | Function-local variables         | `LCL` (RAM[1])→ base address  |
| `argument` | Function arguments               | `ARG` (RAM[2])→ base address  |
| `this`     | Object fields or base pointer    | `THIS` (RAM[3])→ base address |
| `that`     | Secondary object or pointer      | `THAT` (RAM[4])→ base address |
| `temp`     | Temporary variables              | RAM[5] to RAM[12]             |
| `pointer`  | Points to `THIS` and `THAT`      | RAM[3] and RAM[4]             |
| `static`   | Global variables (file-scoped)   | RAM[16+]                      |
| `constant` | Immediate values (not in memory) | N/A (literal values pushed)   |

### Example Operations and Their Translation

#### Stack Manipulation

- `push constant 7` → Load literal `7` and place it at `*SP`, then increment `SP`.
- `pop local 2` → Store the top stack value at the address `LCL + 2`.

#### Arithmetic / Logical Operations

- `add` → Pop two values from the stack, compute the sum, push the result.  
  _Hack equivalent_: uses `AM=M-1`, `D=M`, `M=M+D` with stack pointer management.
- `eq`, `lt`, `gt` → Perform comparison and push `-1` (true) or `0` (false).  
  _Hack equivalent_: uses conditional jump instructions like `JEQ`, `JLT`, `JGT`.

#### Logical Operations

- `and`, `or`, `not` → Perform bitwise operations.  
  _Hack equivalent_: `M=M&D`, `M=M|D`, or `M=!M`.

### Function and Control Flow Support

- **Function Calls**: The VM manages return addresses, arguments, and local variables via the stack and segments like `ARG` and `LCL`.
- **Control Flow**: Labels, `goto`, and `if-goto` map directly to Hack assembly labels and jump instructions.

---

By abstracting these details through the VM layer, high-level code becomes portable and easier to reason about, while the VM translator handles the gritty work of adapting the logic to the Target platform's constraints(→ the Hack platform in this context).

---

This project is part of a broader effort to construct a complete compilation pipeline, from high-level language to binary execution.
