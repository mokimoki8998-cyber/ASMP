# 🚀 ASM+ Programming Language

**ASM+** is a custom, low-level interpreted programming language written in C++. It is designed to bridge the gap between assembly-like syntax and functional high-level logic, providing direct access to memory and real-time console graphics.

## 🛠 Architecture
The interpreter simulates a simplified CPU environment with two primary memory structures:
1.  **Registers/Variables:** Dynamic storage for named integer data.
2.  **Physical RAM (Bit-Addressable):** A dedicated 256-bit (or larger) memory array for storing map data, object states, and collision masks.

---

## 📚 Language Reference (Full Instruction Set)

### 1. Data Manipulation
| Instruction | Syntax | Description |
| :--- | :--- | :--- |
| **MOV** | `MOV <var> <val>` | Assign an integer or another variable's value to `<var>`. |
| **ADD** | `ADD <var> <a> <b>`| Add `a + b` and store the result in `<var>`. |
| **SUB** | `SUB <var> <a> <b>`| Subtract `b` from `a` and store the result in `<var>`. |
| **MUL** | `MUL <var> <a> <b>`| Multiply `a` by `b` and store the result in `<var>`. |
| **DIV** | `DIV <var> <a> <b>`| Perform integer division `a / b` and store in `<var>`. |

### 2. Memory Management (Physical RAM)
| Instruction | Syntax | Description |
| :--- | :--- | :--- |
| **STB** | `STB <idx> <0/1>` | **Set Bit**: Write a bit (0 or 1) to memory address `<idx>`. |
| **GTB** | `GTB <var> <idx>` | **Get Bit**: Read a bit from address `<idx>` into `<var>`. |
| **DMP** | `DMP` | **Dump**: Output the entire state of Physical RAM for debugging. |

### 3. Control Flow
| Instruction | Syntax | Description |
| :--- | :--- | :--- |
| **LABEL** | `:NAME` | Define a marker for jumps and subroutines. |
| **GOTO** | `GOTO :NAME` | Unconditional jump to a label. |
| **IF** | `IF <a> <op> <b> GOTO :N`| Conditional jump (supports `==`, `!=`, `<`, `>`). |
| **GOSUB** | `GOSUB :NAME` | Call a subroutine (pushes return address to stack). |
| **RETURN** | `RETURN` | Return from a subroutine to the last `GOSUB` call. |

### 4. I/O & Graphics
| Instruction | Syntax | Description |
| :--- | :--- | :--- |
| **PXL** | `PXL <x> <y> <char>`| Render an ASCII character at coordinates (x, y). |
| **CLS** | `CLS` | Clear the screen (resets cursor to 0,0). |
| **KEY** | `KEY <var> <code>` | Sets `<var>` to 1 if key `code` is pressed, else 0. |
| **PRINT**| `PRINT <text> %v%` | Output text to console with variable interpolation. |
| **SLEEP**| `SLEEP <ms>` | Pause execution for `<ms>` milliseconds. |

### 5. Advanced Math
| Instruction | Syntax | Description |
| :--- | :--- | :--- |
| **SIN** | `SIN <v> <deg> <s>` | Calculate Sine of `deg` scaled by `s`. |
| **COS** | `COS <v> <deg> <s>` | Calculate Cosine of `deg` scaled by `s`. |
