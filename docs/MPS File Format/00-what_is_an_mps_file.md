# What is an MPS File Format?

An **MPS file** (`.mps`) stands for **Mathematical Programming System**. It is a standard, plain-text file format used to define and archive optimization problems—primarily **Linear Programming (LP)** and **Mixed-Integer Programming (MIP)** problems.

Originally developed by **IBM** in the era of punch cards, it remains an industry standard supported by almost all major optimization solvers, including **Gurobi**, **CPLEX**, **SCIP**, and **COIN-OR**.



## Key Characteristics

### Column-Oriented Representation
*   Unlike human-friendly LP formats that write out full equations (e.g., `2x + 3y <= 6`), MPS specifies the model matrix **variable-by-variable** (column-by-column).

*   This makes it **fast for machines** to parse, though significantly harder for humans to read.

### Fixed vs. Free Format
*   **Fixed Format (Classic):** Data must align with strict column positions (e.g., columns 2, 5, 15, 25) based on legacy punch-card layouts.
  
*   **Free Format (Modern):** Fields are delimited by whitespace, allowing longer variable and constraint names.

## Structure of an MPS File

An `.mps` file is organized into specific text sections. While the order is generally fixed, some sections are optional.

| Section | Purpose |
| :--- | :--- |
| **NAME** | Defines the title of the optimization model. |
| **OBJSENSE** | *(Optional)* Explicitly states whether to `MIN` (minimize) or `MAX` (maximize). |
| **ROWS** | Declares constraint types: `L` ($\le$), `G` ($\ge$), `E` ($=$), and `N` (free/objective function). |
| **COLUMNS** | Lists variable names alongside their matrix non-zero coefficients for each row. |
| **RHS** | Specifies the **Right-Hand-Side** constant vector for the constraints. |
| **BOUNDS** | *(Optional)* Sets lower/upper bounds for variables (e.g., `LO` for lower, `UP` for upper, `BV` for binary). |
| **ENDATA** | Marks the end of the file. |

## Minimal Example

Here is a simple MPS file representing a basic optimization problem:

```mps
NAME          EXAMPLE
ROWS
 N  OBJ
 L  C1
COLUMNS
    X1        OBJ        3
    X1        C1         2
    X2        OBJ        5
    X2        C1         1
RHS
    RHS1      C1        10
BOUNDS
ENDATA
```

### Breakdown of the Example
*   **NAME**: The model is titled "EXAMPLE".
*   **ROWS**:
    *   `N  OBJ`: Defines the objective function row (N = No constraint, just the objective).
    *   `L  C1`: Defines a constraint named "C1" that is "Less than or equal to" ($\le$).
*   **COLUMNS**:
    *   `X1` contributes `3` to the `OBJ` and `2` to `C1`.
    *   `X2` contributes `5` to the `OBJ` and `1` to `C1`.
*   **RHS**: The constant value for constraint `C1` is `10`.
*   **BOUNDS**: Empty in this example (defaults usually apply, e.g., $x \ge 0$).
*   **ENDATA**: Signals the end of the definition.

The mathematical problem represented above is:
*   **Maximize:** $3x_1 + 5x_2$
*   **Subject to:** $2x_1 + 1x_2 \le 10$
*   **Where:** $x_1, x_2 \ge 0$