<div align="center">

<h1>
  <img src="https://img.shields.io/badge/Week%206-DAA%20Lab-00BCD4?style=for-the-badge&logo=c&logoColor=white"/>
</h1>

<h2>〰️ Q3 — Convolution Operation on Vectors of Size n</h2>

<p>
  <img src="https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c"/>
  <img src="https://img.shields.io/badge/Algorithm-FFT%20%28Cooley--Tukey%29-00BCD4?style=flat-square"/>
  <img src="https://img.shields.io/badge/Complexity-O(n%20log%20n)-4CAF50?style=flat-square"/>
  <img src="https://img.shields.io/badge/Naive%20Baseline-O(n²)-FF6B6B?style=flat-square"/>
</p>

</div>

---

## 📌 Overview

This question implements the **convolution of two integer vectors** A (length m) and B (length n, n ≥ m) using an **O(n log n) Divide and Conquer algorithm** based on the **Fast Fourier Transform (FFT)**.

The naive double-loop approach costs **O(n²)** — impractical for large inputs. The FFT-based approach reduces this to **O(n log n)**, providing an asymptotic speedup of O(log n) — over **750× faster** for n = 10,000.

### 🎮 Three Program Modes

| Mode | Invocation | Description |
|------|-----------|-------------|
| 🖥️ Interactive | `./convolution` | Enter your own vectors; every step explained |
| 🔬 Demo | `./convolution demo` | Built-in example: A=[1,2,3], B=[4,5,6,7] |
| 🧪 Test | `./convolution test [N]` | N randomized correctness tests (default: 30,000) |

---

## 📐 What is Convolution?

Given vectors A (length m) and B (length n), the **convolution** C = A ★ B is:

```
C[k] = Σ_{j=0}^{m-1}  A[j] × B[k−j]      for k = 0, 1, ..., m+n−2
```

where B[t] = 0 for t < 0 or t ≥ n. The result C has length **m + n − 1**.

This is **exactly equivalent to multiplying two polynomials:**

```
PA(x) = A[0] + A[1]x + ... + A[m-1]x^(m-1)
PB(x) = B[0] + B[1]x + ... + B[n-1]x^(n-1)

C[k]  =  coefficient of x^k  in  PA(x) × PB(x)
```

---

## ❌ Why the Naive Algorithm Is Insufficient

```
for j in 0..m-1:
    for k in 0..n-1:
        C[j+k] += A[j] × B[k]          ← O(m × n) = O(n²) multiplications
```

For m = n = 10,000 → **100 million** multiplications.  
The FFT approach performs roughly **130,000** operations — **≈ 769× fewer**.

---

## 💡 The Divide and Conquer Idea — Cooley-Tukey FFT

The key algebraic insight:

> A polynomial of degree < N is **uniquely determined** by its values at any N distinct points, and
> `(PA × PB)(xᵢ) = PA(xᵢ) × PB(xᵢ)` — just a **pointwise** product!

So instead of multiplying coefficient-by-coefficient (O(N²)), the algorithm:

<table>
<thead>
  <tr>
    <th style="background:#00BCD4;color:white;">Step</th>
    <th style="background:#00BCD4;color:white;">Action</th>
    <th style="background:#00BCD4;color:white;">Cost</th>
  </tr>
</thead>
<tbody>
  <tr><td>1</td><td>Compute N = smallest power of 2 ≥ (m + n − 1)</td><td>O(log n)</td></tr>
  <tr><td>2</td><td>Zero-pad A and B to length N; copy into complex arrays</td><td>O(N)</td></tr>
  <tr><td>3</td><td><b>FFT(A)</b> — evaluate PA at all N roots of unity</td><td>O(N log N)</td></tr>
  <tr><td>4</td><td><b>FFT(B)</b> — evaluate PB at all N roots of unity</td><td>O(N log N)</td></tr>
  <tr><td>5</td><td><b>Pointwise multiply:</b> A[k] ×= B[k] for k = 0..N−1</td><td>O(N)</td></tr>
  <tr><td>6</td><td><b>Inverse FFT(A)</b> — interpolate back to coefficients</td><td>O(N log N)</td></tr>
  <tr><td>7</td><td>Round real parts of first (m+n−1) entries to integers</td><td>O(N)</td></tr>
</tbody>
</table>

**Total:** `O(N log N) = O(n log n)` since N = Θ(n)

---

## 🌀 The Divide and Conquer Recurrence

Split the polynomial's N coefficients into even and odd halves:

```
P(x) = P_even(x²)  +  x × P_odd(x²)
```

P_even and P_odd each have N/2 coefficients. The **butterfly combine step** is:

```
P(ωₙᵏ)         = P_even(ω_{N/2}ᵏ)  +  ωₙᵏ × P_odd(ω_{N/2}ᵏ)
P(ωₙ^(k+N/2))  = P_even(ω_{N/2}ᵏ)  −  ωₙᵏ × P_odd(ω_{N/2}ᵏ)
```

**Recurrence:** `T(N) = 2·T(N/2) + O(N)` → **T(N) = O(N log N)**

by the Master Theorem: a = 2, b = 2, f(N) = O(N) = Θ(N^log₂2) → Case 2.

---

## 🔧 Two FFT Implementations

### 🟢 Iterative FFT (used for actual convolution — fastest)

1. **Bit-reversal permutation** — places each coefficient at the position the recursive even/odd splitting would reach at the base case.
2. **Butterfly passes** — for len = 2, 4, 8, ..., N: merge two transforms of size len/2 into one of size len in O(len) → O(N) per level → O(N log N) total.

This is **mathematically identical** to the recursive version but avoids function-call overhead for the best constant factor.

### 🔵 Recursive FFT (pedagogical — makes divide & conquer explicit)

```c
fft_recursive(a, N):
    if N == 1: return                         // base case
    even = a[0], a[2], a[4], ...             // divide
    odd  = a[1], a[3], a[5], ...
    fft_recursive(even, N/2)                  // solve left half
    fft_recursive(odd,  N/2)                  // solve right half
    butterfly_combine(even, odd, a, N)        // conquer: O(N)
```

---

## 📊 Complexity Comparison

<table>
<thead>
  <tr>
    <th style="background:#00BCD4;color:white;">Approach</th>
    <th style="background:#00BCD4;color:white;">Time (Worst Case)</th>
    <th style="background:#00BCD4;color:white;">Extra Space</th>
  </tr>
</thead>
<tbody>
  <tr><td>❌ Naive double loop</td><td><code>O(m × n) = O(n²)</code></td><td>O(m + n) for output</td></tr>
  <tr><td>✅ FFT Divide &amp; Conquer</td><td><b><code>O(n log n)</code></b></td><td>O(n) complex numbers</td></tr>
</tbody>
</table>

---

## 🧪 Correctness Verification

After each FFT convolution, the result is **cross-checked against the naive oracle** when m×n ≤ 4,000,000. For larger inputs, the 30,000-case test harness handles validation.

The **test mode** covers three size classes:

| Class | Proportion | Size Range |
|-------|-----------|-----------|
| Small | 80% | m ≤ 20, n ≤ 25 |
| Medium | 17% | m ≤ 200, n ≤ 300 |
| Large (stress) | 3% | m ≤ 800, n ≤ m+500 |

Every FFT result is compared **exactly** (integer equality) against the naive oracle. `max_abs_err` should always be 0 for integer inputs of reasonable magnitude.

---

## 🔢 Numeric Considerations

- FFT operates over **complex `double`** (C99 `double complex`).
- Integer coefficients loaded as `double`, processed through FFT.
- Real parts of results rounded via `llround()` — exact for integer inputs within `double` precision range (~15 significant digits).

---

## 📂 Files in This Folder

| File | Description |
|------|-------------|
| `q3_convolution_operation_on_vectors_of_size_n.c` | Main C source — FFT convolution with both iterative and recursive implementations |
| `q3_convolution_operation_on_vectors_of_size_n.exe` | Compiled executable |
| `convolution_complexity_analysis.png` | Timing graph: naive O(n²) vs FFT O(n log n) |

---

## 🛠️ How to Compile and Run

```bash
# Compile
gcc -O2 -Wall -std=c11 q3_convolution_operation_on_vectors_of_size_n.c -lm -o q3

# Interactive (enter your own vectors)
./q3

# Demo (A=[1,2,3], B=[4,5,6,7])
./q3 demo

# Run 30000 correctness tests
./q3 test 30000
```

---

## 🔑 Key Design Decisions

| Decision | Reason |
|----------|--------|
| Iterative FFT for production | Avoids function-call overhead; same O(N log N), better constant factor |
| Recursive FFT kept for pedagogy | Makes the divide-and-conquer structure directly visible in code |
| Zero-padding to next power of 2 | Required for Cooley-Tukey butterfly structure |
| `calloc` for FFT arrays | Auto-zeros padding region — no separate `memset` needed |
| `llround()` for final rounding | Eliminates floating-point noise for integer coefficient convolutions |
| Early cross-check skip for large m×n | The naive oracle itself would be slow for m×n > 4M |

---

<div align="center">
<sub>📘 DAA Lab · Week 6 · Q3 · Convolution via FFT Divide &amp; Conquer</sub>
</div>
