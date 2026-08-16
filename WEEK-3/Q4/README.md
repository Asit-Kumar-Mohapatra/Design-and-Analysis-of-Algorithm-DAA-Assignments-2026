<style>
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Rajdhani:wght@400;500;600;700&family=Fira+Code:wght@300;400;500&family=Inter:wght@300;400;500;600&display=swap');
* { box-sizing: border-box; }
.hud { background: #020408; background-image: radial-gradient(ellipse 80% 50% at 20% 20%, rgba(0,210,255,0.07) 0%, transparent 60%), radial-gradient(ellipse 60% 40% at 80% 80%, rgba(160,0,255,0.07) 0%, transparent 60%), linear-gradient(rgba(0,210,255,0.025) 1px, transparent 1px), linear-gradient(90deg, rgba(0,210,255,0.025) 1px, transparent 1px); background-size: auto, auto, 40px 40px, 40px 40px; color: #b0bec5; font-family: 'Inter', sans-serif; padding: 60px 50px; border-radius: 20px; position: relative; overflow: hidden; line-height: 1.75; }
.hud::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 3px; background: linear-gradient(90deg, transparent, #00d2ff, #a000ff, #00d2ff, transparent); animation: border-run 4s linear infinite; }
@keyframes border-run { 0%{background-position:-200% 0} 100%{background-position:200% 0} }
.hud-inner { position: relative; z-index: 1; }
.header { text-align: center; padding: 20px 0 40px; animation: fadeDown 0.8s ease both; }
.q-label { font-family: 'Fira Code', monospace; font-size: 0.9em; color: #00d2ff; letter-spacing: 6px; text-transform: uppercase; opacity: 0.8; margin-bottom: 8px; }
.title { font-family: 'Orbitron', sans-serif; font-size: 3.4em; font-weight: 900; letter-spacing: 3px; background: linear-gradient(135deg, #00d2ff 0%, #ffffff 40%, #a000ff 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin: 0; line-height: 1.2; filter: drop-shadow(0 0 20px rgba(0,210,255,0.4)); }
.subtitle { font-family: 'Rajdhani', sans-serif; font-size: 1.3em; color: #ff2d87; letter-spacing: 5px; text-transform: uppercase; margin-top: 8px; font-weight: 600; }
.badges { display: flex; flex-wrap: wrap; justify-content: center; gap: 12px; margin: 30px 0 50px; }
.badge { font-family: 'Fira Code', monospace; font-size: 0.85em; padding: 8px 20px; border-radius: 3px; border: 1px solid rgba(0,210,255,0.5); color: #00d2ff; background: rgba(0,210,255,0.05); letter-spacing: 2px; transition: all 0.3s; }
.badge:hover { background: rgba(0,210,255,0.15); transform: translateY(-2px); box-shadow: 0 0 20px rgba(0,210,255,0.3); }
.divider { display: flex; align-items: center; gap: 15px; margin: 50px 0 30px; }
.divider-line { flex: 1; height: 1px; background: linear-gradient(to right, transparent, rgba(0,210,255,0.4), transparent); }
.divider-label { font-family: 'Orbitron', sans-serif; font-size: 0.75em; color: rgba(0,210,255,0.6); letter-spacing: 4px; text-transform: uppercase; white-space: nowrap; }
.grid { display: grid; gap: 24px; margin-bottom: 24px; }
.grid-2 { grid-template-columns: repeat(2, 1fr); }
@media(max-width:800px){.grid-2{grid-template-columns:1fr;}}
.panel { background: rgba(8,14,24,0.7); border: 1px solid rgba(255,255,255,0.05); border-radius: 14px; padding: 30px; position: relative; transition: transform 0.4s cubic-bezier(.23,1,.32,1), box-shadow 0.4s; animation: slideUp 0.8s ease both; backdrop-filter: blur(10px); }
.panel:nth-child(1){animation-delay:0.1s}.panel:nth-child(2){animation-delay:0.2s}.panel:nth-child(3){animation-delay:0.3s}
.panel::before,.panel::after { content:''; position:absolute; width:16px; height:16px; transition:all 0.3s; }
.panel::before { top:0; left:0; border-top:2px solid #00d2ff; border-left:2px solid #00d2ff; border-radius:14px 0 0 0; }
.panel::after  { bottom:0; right:0; border-bottom:2px solid #a000ff; border-right:2px solid #a000ff; border-radius:0 0 14px 0; }
.panel:hover { transform: translateY(-8px); box-shadow: 0 24px 60px rgba(0,0,0,0.6), 0 0 30px rgba(0,210,255,0.08); }
.panel:hover::before { width:30px; height:30px; border-color:#ff2d87; }
.panel:hover::after  { width:30px; height:30px; border-color:#00d2ff; }
.panel.full { grid-column: 1/-1; }
.panel.accent-cyan  { border-top: 2px solid rgba(0,210,255,0.5); }
.panel.accent-purple{ border-top: 2px solid rgba(160,0,255,0.5); }
.panel.accent-pink  { border-top: 2px solid rgba(255,45,135,0.5); }
.panel h2 { font-family: 'Orbitron', sans-serif; font-size: 1.15em; font-weight: 700; color: #fff; margin: 0 0 18px; letter-spacing: 1px; display:flex; align-items:center; gap:10px; }
.panel h2 .icon { color: #ff2d87; font-size: 0.7em; }
.panel h3 { font-family: 'Rajdhani', sans-serif; font-size: 1.2em; font-weight: 700; color: #00d2ff; margin: 22px 0 8px; letter-spacing: 2px; text-transform: uppercase; border-bottom: 1px dashed rgba(0,210,255,0.25); padding-bottom: 6px; }
.panel p { font-size: 1.0em; color: #b0bec5; margin: 0 0 14px; }
.panel ul, .panel ol { padding-left: 20px; margin: 0 0 14px; }
.panel li { font-size: 0.98em; color: #b0bec5; margin-bottom: 8px; line-height: 1.7; }
.panel li b { color: #e0e8f0; }
code { font-family:'Fira Code',monospace; font-size:0.87em; background:rgba(0,0,0,0.6); color:#00d2ff; padding:2px 7px; border-radius:4px; border:1px solid rgba(0,210,255,0.2); }
.term { background:#000; border-radius:8px; overflow:hidden; margin:16px 0; border:1px solid rgba(255,255,255,0.08); box-shadow: inset 0 0 20px rgba(0,0,0,0.5); }
.term-bar { background:#1a1a1a; padding:8px 14px; display:flex; align-items:center; gap:7px; border-bottom:1px solid rgba(255,255,255,0.06); }
.term-dot { width:11px; height:11px; border-radius:50%; }
.term-dot.r{background:#ff5f57}.term-dot.y{background:#febc2e}.term-dot.g{background:#28c840}
.term-body { padding:16px 20px; font-family:'Fira Code',monospace; font-size:0.92em; line-height:1.7; color:#00ff88; white-space:pre; overflow-x:auto; }
.term-body .c{color:#666}.term-body .k{color:#00d2ff}.term-body .s{color:#ff2d87}
.formula { background: linear-gradient(135deg, rgba(0,0,0,0.8), rgba(10,5,20,0.8)); border:1px solid rgba(160,0,255,0.3); border-radius:8px; padding:16px 24px; text-align:center; font-family:'Fira Code',monospace; font-size:1.05em; color:#d0a0ff; letter-spacing:1.5px; margin:14px 0; box-shadow: 0 0 20px rgba(160,0,255,0.1); }
.callout { display:flex; gap:14px; align-items:flex-start; background:rgba(255,45,135,0.05); border:1px solid rgba(255,45,135,0.2); border-radius:8px; padding:16px 20px; margin:16px 0; }
.callout-icon{font-size:1.3em;line-height:1;flex-shrink:0;margin-top:2px;}
.callout p{color:#e0cce8;margin:0;font-size:0.97em;}
.steps{counter-reset:step;list-style:none;padding:0;margin:0;}
.steps li{counter-increment:step;position:relative;padding:14px 14px 14px 56px;border-left:2px solid rgba(0,210,255,0.15);margin-bottom:0;}
.steps li::before{content:counter(step);position:absolute;left:-16px;top:14px;width:30px;height:30px;background:rgba(0,210,255,0.15);border:1px solid rgba(0,210,255,0.4);border-radius:50%;color:#00d2ff;font-family:'Orbitron',sans-serif;font-size:0.75em;display:flex;align-items:center;justify-content:center;font-weight:700;}
.steps li:last-child{border-color:rgba(160,0,255,0.3);}
.steps li:last-child::before{background:rgba(160,0,255,0.15);border-color:rgba(160,0,255,0.4);color:#a000ff;}
.graph-stage{perspective:1400px;display:flex;justify-content:center;margin:50px 0;animation:fadeIn 1.2s ease both 0.3s;}
.graph-card{width:88%;max-width:820px;transform-style:preserve-3d;animation:levitate 9s ease-in-out infinite alternate;position:relative;}
.graph-card img{width:100%;border-radius:10px;border:1px solid rgba(160,0,255,0.4);box-shadow:0 40px 80px rgba(0,0,0,0.9),0 0 50px rgba(0,210,255,0.15);transform:translateZ(60px);display:block;}
.graph-card::after{content:'';position:absolute;inset:-12px;border-radius:20px;background:linear-gradient(135deg,rgba(0,210,255,0.3),rgba(160,0,255,0.3));transform:translateZ(-25px);filter:blur(30px);opacity:0.5;animation:glow-pulse 4s ease-in-out infinite alternate;}
@keyframes levitate{from{transform:rotateX(6deg) rotateY(-6deg) translateY(0px);}to{transform:rotateX(-4deg) rotateY(6deg) translateY(-18px);}}
@keyframes glow-pulse{from{opacity:0.3;}to{opacity:0.7;}}
@keyframes fadeDown{from{opacity:0;transform:translateY(-25px);}to{opacity:1;transform:none;}}
@keyframes fadeIn{from{opacity:0;transform:scale(0.95);}to{opacity:1;transform:scale(1);}}
@keyframes slideUp{from{opacity:0;transform:translateY(35px);}to{opacity:1;transform:none;}}
</style>

<div class="hud">
<div class="hud-inner">

<div class="header">
  <div class="q-label">// Week-3 · Question 04 · DAA Lab</div>
  <h1 class="title">STRASSEN'S MATRIX</h1>
  <div class="subtitle">O(N^2.807) · HPC Divide & Conquer Multiplication</div>
</div>

<div class="badges">
  <span class="badge">TIME: O(N^2.807)</span>
  <span class="badge">SPACE: O(N^2)</span>
  <span class="badge">THRESHOLD: 16</span>
  <span class="badge">LANG: C</span>
  <span class="badge">HPC D&C</span>
</div>

<div class="graph-stage">
  <div class="graph-card">
    <img src="./strassen_matrix_multiplication_graph.png" alt="Strassen Matrix Complexity">
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Multiply two square <b>N×N matrices A and B</b> using Strassen's Divide and Conquer algorithm. The input N must be a <b>power of 2</b> (e.g., 2, 4, 8, 16, 32...). Compare the time complexity of Strassen's method against naive matrix multiplication and prove the improvement using the Master Theorem.</p>
    <p>Implement an HPC-grade version in C that uses: a <b>hybrid base-case threshold</b>, <b>zero-copy strided submatrix views</b>, and a <b>pre-allocated workspace buffer</b> to avoid heap fragmentation during recursion.</p>
    <div class="callout">
      <div class="callout-icon">💡</div>
      <p><b>Strassen's Insight (1969):</b> Standard matrix multiplication of 2x2 blocks requires 8 multiplications. Volker Strassen showed that using clever algebraic identities, you can do it with just 7 multiplications. This seemingly tiny saving compounds exponentially in the recursive algorithm, ultimately delivering a sub-cubic time complexity.</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Algorithm Deep Dive</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> STRASSEN'S 7 FORMULAS</h2>
    <p>Each N×N matrix is split into 4 quadrants of size N/2 × N/2: A11, A12, A21, A22 and B11, B12, B21, B22. The 7 products are:</p>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body">M1 = (A11 + A22) * (B11 + B22)
M2 = (A21 + A22) * B11
M3 = A11 * (B12 - B22)
M4 = A22 * (B21 - B11)
M5 = (A11 + A12) * B22
M6 = (A21 - A11) * (B11 + B12)
M7 = (A12 - A22) * (B21 + B22)</div>
    </div>
    <p>Reconstruct the result quadrants from these 7 products using only additions:</p>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body">C11 = M1 + M4 - M5 + M7
C12 = M3 + M5
C21 = M2 + M4
C22 = M1 - M2 + M3 + M6</div>
    </div>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> MASTER THEOREM PROOF</h2>
    <h3>Standard D&C (8 multiplications)</h3>
    <div class="formula">T(n) = 8·T(n/2) + O(n²)
log₂(8) = 3  →  O(n³)</div>
    <h3>Strassen (7 multiplications)</h3>
    <div class="formula">T(n) = 7·T(n/2) + O(n²)</div>
    <p>Applying Master Theorem: a=7, b=2, f(n)=n²</p>
    <ul>
      <li>Critical exponent: <code>log₂(7) ≈ 2.807</code></li>
      <li>Since <code>f(n) = n² = O(n^2.807)</code>, Case 1 applies.</li>
    </ul>
    <div class="formula">Time = O(n^(log₂7)) ≈ O(n^2.807)</div>
    <p>For N=1024: Standard = ~10⁹ ops; Strassen = ~10^8.4 ops — ~4× faster!</p>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// C Code Walkthrough</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> THREE CRITICAL OPTIMIZATIONS IN THE C CODE</h2>
    <div class="grid grid-2">
      <div>
        <h3>1. HYBRID THRESHOLD</h3>
        <div class="term">
          <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
          <div class="term-body">#define THRESHOLD 16

<span class="k">if</span> (n <= THRESHOLD) {
  standardMultiplyStrided(...);
  <span class="k">return</span>;
}</div>
        </div>
        <p>When N ≤ 16, recursion overhead exceeds the savings. Fallback to a cache-optimized <code>i-k-j</code> loop which fits entire 16×16 submatrices in L1 cache, maximizing SIMD vectorization.</p>
      </div>
      <div>
        <h3>2. ZERO-COPY STRIDED VIEWS</h3>
        <div class="term">
          <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
          <div class="term-body"><span class="c">// No copy — just pointer + stride</span>
<span class="k">const int</span>* A11 = A;
<span class="k">const int</span>* A12 = A + k;
<span class="k">const int</span>* A21 = A + k * lda;
<span class="k">const int</span>* A22 = A + k * lda + k;</div>
        </div>
        <p>Submatrix "views" are created with zero memory copying. Each quadrant is just a pointer into the parent matrix, with the stride (lda) used to advance rows.</p>
      </div>
    </div>
    <div style="margin-top:20px;">
      <h3>3. PRE-ALLOCATED WORKSPACE BUFFER</h3>
      <div class="term">
        <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
        <div class="term-body"><span class="c">// Compute total workspace needed across all recursion levels</span>
size_t workspaceSize = 0;
<span class="k">for</span> (tempN = n/2; tempN >= THRESHOLD; tempN /= 2)
    workspaceSize += 9 * tempN * tempN; <span class="c">// 7 Mi + 2 temp</span>

<span class="k">int</span>* workspace = (<span class="k">int</span>*)calloc(workspaceSize, <span class="k">sizeof</span>(<span class="k">int</span>));

<span class="c">// Inside recursion: slice workspace into buffers</span>
<span class="k">int</span>* M1 = workspace;
<span class="k">int</span>* M2 = M1 + kSquare;  <span class="c">// ... and so on</span>
<span class="k">int</span>* nextWorkspace = t2 + kSquare; <span class="c">// pass remainder deeper</span></div>
      </div>
      <p>All 7 intermediate M matrices and 2 temporary buffers are sliced from a single pre-allocated block. Zero dynamic <code>malloc()</code> calls occur during recursion — eliminating heap contention, fragmentation, and allocation latency in deep recursive calls.</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Graph Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> WHAT THE GRAPH SHOWS</h2>
    <p>The <code>strassen_matrix_multiplication_graph.png</code> plots <b>operation count vs N</b> for both algorithms.</p>
    <ul>
      <li><b>Initial crossover:</b> For small N (typically N &lt; 64), Standard is faster — Strassen's 18 matrix additions overhead is not worth the 1 saved multiplication.</li>
      <li><b>The inflection point:</b> As N crosses the crossover, Strassen's sub-cubic growth pulls dramatically ahead.</li>
      <li><b>Divergence:</b> At N=256, Standard is ~4× slower. At N=1024, Standard is ~10× slower. The gap compounds exponentially with N.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> POWER OF 2 CONSTRAINT</h2>
    <p>The implementation requires N = 2^k. If the user enters a non-power-of-2, the program outputs a clear explanation and exits gracefully:</p>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">bool</span> isPowerOfTwo(<span class="k">int</span> n) {
  <span class="k">return</span> (n > 0) && ((n & (n-1)) == 0);
}</div>
    </div>
    <p><b>Why?</b> Strassen requires splitting each dimension exactly in half to create equal-sized quadrants at every level. If N is not a power of 2, the quadrant dimensions become fractional — breaking the recursion.</p>
    <div class="callout">
      <div class="callout-icon">📌</div>
      <p>Production implementations handle arbitrary N by <b>zero-padding</b> matrices to the next power of 2 before applying Strassen, then cropping the result.</p>
    </div>
  </div>
</div>

</div>
</div>
