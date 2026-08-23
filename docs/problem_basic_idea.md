
## What Is the Actual Problem?

Industries such as refineries have a huge number of decisions to make.

For example, MRPL may need to decide:

- How much crude oil should be purchased?
- Which type of crude should go into which refinery unit?
- How much petrol, diesel, and jet fuel should be produced?
- Which machines or units should be operated?
- How should electricity be distributed?
- Which products should be transported to which locations?
- How can all of these decisions be made at **minimum cost** or **maximum profit**?

### Problem Category

This is an **optimization problem**.

---

## What Does the Problem Look Like?

At a high level, the objective is:

> **Find the best possible values for thousands or millions of variables while satisfying thousands or millions of constraints.**

Mathematically, an optimization problem can be represented as:

```text
                Optimization Problem
                        │
          ┌─────────────┼─────────────┐
          │             │             │
          ▼             ▼             ▼
      Variables      Constraints    Objective
          │             │             │
          │             │             │
       What can      What must      What are
       we change?    be satisfied?  we optimizing?
````

For example:

```text
Variables:
    crude purchased
    diesel produced
    electricity consumed
    units operated
    products transported
    ...

Constraints:
    refinery capacity
    production limits
    material balance
    market demand
    storage capacity
    equipment availability
    ...

Objective:
    maximize profit
    minimize cost
    minimize energy consumption
    maximize production
    ...
```

---

## Existing Solutions

Commercial optimization solvers such as **CPLEX, Gurobi, and Xpress** are already capable of solving many of these problems.

However, these solutions are:

* Commercial and license-based
* Expensive at large scale
* Closed-source
* Difficult to modify internally
* Not specifically designed around India's strategic industrial requirements

### The Challenge

The goal of this project is to develop a **sovereign optimization solver** instead of depending entirely on commercial optimization software.

The solver should be developed **from mathematical foundations**, rather than being built on an existing open-source optimization solver.

---

## What Does "Solver" Mean?

Think of an optimization problem as something we give to a mathematical engine.

### 1. Variables

Variables represent the decisions that the solver is allowed to make.

For example:

```text
crude amount       = 100 tonnes
diesel production  = 50 tonnes
electricity usage  = 20 MW
```

In a real industrial problem, there could be **thousands or millions of such variables**.

---

### 2. Constraints

Constraints represent the rules that must be satisfied.

For example:

```text
Refinery capacity cannot be exceeded.

Minimum production requirements must be satisfied.

Material balance must be maintained.

Customer demand must be satisfied.

Storage capacity cannot be exceeded.

Certain machines cannot operate simultaneously.
```

Mathematically, a constraint might look like:

```text
crude_used <= refinery_capacity
```

or:

```text
production >= minimum_required_production
```

---

### 3. Objective

The objective defines what we are trying to achieve.

For example:

```text
Maximize:
    profit

or

Minimize:
    production cost

or

Minimize:
    fuel consumption
```

---

## So, What Does the Solver Actually Do?

The **solver is the mathematical engine** that takes:

```text
Variables
    +
Constraints
    +
Objective
```

and determines the best possible solution.

In simple terms:

> **The solver finds values for all the decision variables such that all constraints are satisfied while the objective is optimized.**

For example:

```text
                 INPUT MODEL
                     │
                     ▼
          ┌─────────────────────┐
          │       SOLVER        │
          │                     │
          │ Variables           │
          │ Constraints         │
          │ Objective           │
          │                     │
          │ Optimization        │
          │ Algorithms          │
          └──────────┬──────────┘
                     │
                     ▼
               BEST SOLUTION
                     │
          ┌──────────┼──────────┐
          ▼          ▼          ▼
       Crude      Production   Energy
       Plan        Plan         Plan
```

### In one sentence

> **We are building a mathematical optimization engine from scratch that can find high-quality solutions to extremely large and difficult industrial optimization problems.**

At the end, you should have a **working optimization solver software**, not just documentation or an algorithm demo.

Think of the final output like this:

```text
                    YOUR PROJECT
                         │
                         ▼
             ┌──────────────────────┐
             │ Indigenous Optimizer │
             │      Solver          │
             └──────────┬───────────┘
                        │
          ┌─────────────┼─────────────┐
          ▼             ▼             ▼
         LP            MILP           QP
          │             │             │
          └─────────────┼─────────────┘
                        ▼
                Optimization Engine
                        │
             ┌──────────┴──────────┐
             ▼                     ▼
           CPU                    GPU
       computation            acceleration
             │                     │
             └──────────┬──────────┘
                        ▼
                   Best Solution
```

### Concretely, the final deliverables are:

1. **A solver executable/library**

   * e.g. `solver`
   * Accepts an optimization model.
   * Solves it.
   * Returns the solution.

2. **LP support**

   * Linear Programming
   * Revised Simplex and/or Interior Point methods.

3. **MILP support**

   * Mixed-Integer Linear Programming
   * Branch-and-Bound
   * Branch-and-Cut
   * Cutting planes
   * Heuristics
   * Node selection.

4. **QP support**

   * Quadratic Programming.

5. **Numerical engine**

   * Sparse matrices
   * Numerical stability
   * Scaling
   * Factorization
   * Feasibility and optimality checks.

6. **GPU acceleration**

   * GPU is used for operations where it provides a measurable speedup.
   * You should demonstrate **CPU vs GPU performance**.

7. **Benchmark results**

   * MIPLIB
   * Netlib
   * Mittelmann
   * QPLIB

8. **Industrial demonstrations**

   * Refinery scheduling
   * Crude blending
   * Production planning
   * Logistics
   * Power dispatch

9. **Comparison**

For example:

```text
                    Benchmark
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
      Your Solver    HiGHS       CPLEX/Gurobi
          │            │            │
          ▼            ▼            ▼
       42 sec        35 sec        18 sec
```

The goal isn't necessarily to beat every commercial solver on every problem. You need to demonstrate **correctness, robustness, scalability, and competitive performance**, including cases where your design has an advantage.

### The final proof should basically answer:

> **"Can our solver take a large real optimization problem, solve it correctly and reliably, and do so fast enough to be useful in an industrial environment?"**

So your GitHub repository eventually becomes:

```text
Problem
   ↓
Mathematical Theory
   ↓
Algorithm Design
   ↓
Solver Implementation
   ↓
Testing
   ↓
Benchmarks
   ↓
Industrial Case Studies
   ↓
Performance Comparison
   ↓
FINAL WORKING SOVEREIGN SOLVER


