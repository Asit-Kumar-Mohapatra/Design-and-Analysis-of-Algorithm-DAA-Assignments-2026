<div align="center">

<h1>
  <img src="https://img.shields.io/badge/Week%206-DAA%20Lab-0D6EFD?style=for-the-badge&logo=c&logoColor=white"/>
</h1>

<h2>🔢 Q2 — 2D Square Matrix Operations &amp; Their Worst-Case Complexities</h2>

<p>
  <img src="https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c"/>
  <img src="https://img.shields.io/badge/Operations-7-0D6EFD?style=flat-square"/>
  <img src="https://img.shields.io/badge/Max%20Complexity-O(n³)-FF6B6B?style=flat-square"/>
  <img src="https://img.shields.io/badge/Storage-Row--Major%201D%20Array-4CAF50?style=flat-square"/>
</p>

</div>

---

## 📌 Overview

This question implements **seven fundamental operations** on `n × n` square matrices of `double` values, analyzes the worst-case time complexity of each, and justifies why each algorithm is **optimal** for the general dense case (no special structure assumed).

> **Storage Layout:** The matrix is stored as a **single flattened 1D array** of length `n²`.
> Element at row `i`, column `j` → accessed as `A[i*n + j]`.
>
> **Why:** One `malloc()`, contiguous memory, excellent cache locality on row-major scans, zero pointer-chasing overhead.

### 🎮 Three Program Modes

| Mode | Invocation | Description |
|------|-----------|-------------|
| 🖥️ Interactive | `./matrix_ops` | Enter your own matrix; every operation runs with a full explanation |
| 🔬 Demo | `./matrix_ops demo` | Built-in 3×3 symmetric positive-definite example |
| 🧪 Test | `./matrix_ops test [N]` | N randomized correctness tests (default: 30,000) |

---

## ⚡ Operations at a Glance

<table>
<thead>
  <tr>
    <th style="background:#0D6EFD;color:white;">#</th>
    <th style="background:#0D6EFD;color:white;">Operation</th>
    <th style="background:#0D6EFD;color:white;">Time (Worst Case)</th>
    <th style="background:#0D6EFD;color:white;">Extra Space</th>
  </tr>
</thead>
<tbody>
  <tr><td><b>i</b></td><td>Matrix Addition / Subtraction</td><td><code>Θ(n²)</code></td><td>O(n²) or O(1)*</td></tr>
  <tr><td><b>ii</b></td><td>Matrix Multiplication</td><td><code>O(n³)</code></td><td>O(n²)</td></tr>
  <tr><td><b>iii</b></td><td>Is Zero Matrix?</td><td><code>O(n²)</code></td><td>O(1)</td></tr>
  <tr><td><b>iv</b></td><td>Is Symmetric Matrix?</td><td><code>O(n²)</code></td><td>O(1)</td></tr>
  <tr><td><b>v</b></td><td>Determinant (LU Decomposition)</td><td><code>O(n³)</code></td><td>O(n²)</td></tr>
  <tr><td><b>vi</b></td><td>Transpose In-Place</td><td><code>Θ(n²)</code></td><td>O(1)</td></tr>
  <tr><td><b>vii</b></td><td>Eigenvalues &amp; Eigenvectors</td><td><code>O(k·n³) / O(k·n²)</code></td><td>O(n²)</td></tr>
</tbody>
</table>

<sub>*O(1) if done in-place as A += B; O(n²) if a separate output matrix C is produced.</sub>

---

## 🔬 Detailed Analysis

### 🔵 (i) Matrix Addition — `Θ(n²)`

| | |
|---|---|
| **Lower Bound** | n² output cells, each needing exactly one addition. Cannot produce n² results from fewer than n² inputs → **Ω(n²)** (tight, making it Θ) |
| **Algorithm** | Single linear pass over all n² elements: `C[k] = A[k] + B[k]` |
| **Optimal?** | ✅ Yes — Θ(n²): both a lower *and* upper bound |

Subtraction uses the identical approach with the same complexity.

---

### 🔴 (ii) Matrix Multiplication — `O(n³)`

| | |
|---|---|
| **Lower Bound** | n² output cells × n multiplications each = n³ scalar multiplications |
| **Algorithm** | **i-k-j loop order** (not the textbook i-j-k) |
| **Why i-k-j?** | Inner loop scans B and C **row-wise** (stride-1 memory) → dramatically better cache performance than i-j-k which strides through B by n per step |
| **Optimal?** | ✅ Yes for practical use. Strassen (O(n^2.807)) and CW-family (O(n^2.371)) exist theoretically but carry large constants and numerical instability — not worthwhile at typical problem sizes |

```
For each row i:
  For each column k of A:       ← fixes a_ik scalar
    For each column j:           ← inner loop: row-wise access of B and C
      C[i][j] += a_ik × B[k][j]
```

---

### 🔵 (iii) Is Zero Matrix — `O(n²)` Worst Case

| | |
|---|---|
| **Lower Bound** | A zero cannot be confirmed without reading the entry. Last entry might be the only non-zero → O(n²) worst case |
| **Algorithm** | Linear scan with **early exit** on first non-zero entry |
| **Best Case** | O(1) — if `A[0][0] ≠ 0`, returns immediately |

---

### 🔵 (iv) Is Symmetric Matrix — `O(n²)` Worst Case

| | |
|---|---|
| **Lower Bound** | n(n−1)/2 off-diagonal pairs must be checked → Θ(n²) |
| **Algorithm** | Double loop over `i < j` pairs comparing `A[i][j]` with `A[j][i]`, with early exit on mismatch |
| **Best Case** | O(1) — if the very first checked pair mismatches |

---

### 🔴 (v) Determinant via LU Decomposition — `O(n³)`

| | |
|---|---|
| **Rejected approach** | Cofactor/Laplace expansion → **O(n!)** — catastrophically slow |
| **Algorithm** | **Gaussian Elimination with Partial Pivoting** |
| **Optimal?** | ✅ Yes — O(n³) is standard practical optimum for general dense matrices |

**How it works:**

```
1. For each column c = 0 to n-1:
   a. Find the largest-magnitude entry in column c at or below row c  (partial pivot)
   b. Swap that row with row c; flip the sign of det
   c. det *= A[c][c]
   d. Eliminate all entries below A[c][c] using row operations
2. det(A) = (−1)^(#swaps) × product of diagonal of U
```

**Why partial pivoting?** Choosing the largest pivot avoids dividing by very small numbers (which amplifies floating-point error). It does **not** change the O(n³) bound — only improves numerical stability.

> The naive cofactor expansion is kept in the code *only* as a correctness oracle for the test harness on small matrices (n ≤ 6).

---

### 🔵 (vi) Transpose In-Place — `Θ(n²)` Time, `O(1)` Space

| | |
|---|---|
| **Lower Bound** | Every off-diagonal element must move → **Ω(n²)** |
| **Algorithm** | For every pair (i, j) with i < j: swap `A[i][j]` ↔ `A[j][i]`. Diagonal stays untouched. Exactly n(n−1)/2 swaps. |
| **Extra Space** | Only a single scalar `tmp` variable — **O(1)** |
| **Optimal?** | ✅ Yes — both time Θ(n²) and space O(1) are the best achievable |

---

### 🟠 (vii) Eigenvalues and Eigenvectors — `O(k·n³)` / `O(k·n²)`

> ⚠️ **No exact closed-form algorithm exists** for n ≥ 5 by the **Abel-Ruffini theorem** (polynomials of degree ≥ 5 have no solution by radicals). Eigenvalues *must* be computed iteratively. There is no fixed worst-case bound purely in n — only O(k × per-iteration cost), where k depends on data and required precision.

**Two algorithms are implemented:**

#### 🔹 (a) Jacobi Eigenvalue Algorithm — Symmetric Matrices Only

| Property | Detail |
|----------|--------|
| **Per rotation cost** | O(n) — updates one row and one column |
| **Per sweep cost** | O(n³) — n(n−1)/2 rotations per sweep |
| **Convergence** | Quadratic → O(log log(1/ε)) sweeps for fixed ε |
| **Output** | All eigenvalues AND orthonormal eigenvectors |
| **Total practical cost** | O(n³ × sweeps) ≈ O(n³) for fixed precision |

#### 🔹 (b) Power Iteration — General Matrices (Dominant Eigenpair Only)

| Property | Detail |
|----------|--------|
| **Per iteration cost** | O(n²) — one matrix-vector product |
| **Convergence** | Linear at rate \|λ₂/λ₁\| per iteration |
| **Output** | Single largest-magnitude eigenvalue/eigenvector |
| **Total cost** | O(n² × k) |

---

## 📂 Files in This Folder

| File | Description |
|------|-------------|
| `q2_2D_square_matrix_operations_and_their_complexities.c` | Main C source — all seven operations |
| `q2_2D_square_matrix_operations_and_their_complexities.exe` | Compiled executable |
| `matrix_complexity_analysis.png` | Timing graph: measured vs. theoretical complexities |

---

## 🛠️ How to Compile and Run

```bash
# Compile
gcc -O2 -Wall -std=c11 q2_2D_square_matrix_operations_and_their_complexities.c -lm -o q2

# Interactive mode
./q2

# Demo mode (no input needed)
./q2 demo

# Test mode
./q2 test 30000
```

---

## 🔑 Key Design Decisions

| Decision | Reason |
|----------|--------|
| Flattened 1D array `A[i*n+j]` | Superior cache locality vs. `double**` pointer grid |
| i-k-j loop order | Stride-1 inner loop — dramatically better cache performance |
| Partial pivoting in LU | Numerical stability without changing O(n³) asymptotic |
| Symmetrized copy for Jacobi | Jacobi requires symmetric input; only the copy is symmetrized, not the original |
| Independent reference implementations | Cross-validation of multiplication and determinant in the test harness |

---

<div align="center">
<sub>📘 DAA Lab · Week 6 · Q2 · 2D Square Matrix Operations</sub>
</div>
