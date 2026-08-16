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
  <div class="q-label">// Week-3 · Question 05 · DAA Lab</div>
  <h1 class="title">SPECIAL MATRICES</h1>
  <div class="subtitle">Symmetric Pattern Multiplication · O(N²) via D&C</div>
</div>

<div class="badges">
  <span class="badge">TIME: O(N²)</span>
  <span class="badge">SPACE: O(N²)</span>
  <span class="badge">ONLY 2 RECURSIVE CALLS</span>
  <span class="badge">LANG: C</span>
  <span class="badge">D&C</span>
</div>

<div class="graph-stage">
  <div class="graph-card">
    <img src="./Matrix_Complexity_Analysis.png" alt="Special Matrix Complexity">
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Some special square matrices possess a symmetric block structure. Specifically, a <b>"Special Pattern Matrix"</b> M of size N×N has the following 2×2 block form (where each block itself is an (N/2)×(N/2) matrix):</p>
    <div class="formula">M = [ M₁  M₂ ]
    [ M₂  M₁ ]</div>
    <p>The off-diagonal blocks are identical, and so are the diagonal blocks. Exploit this symmetry to multiply two such matrices A and B using a <b>Divide and Conquer approach with only 2 recursive multiplications</b> instead of the standard 4, achieving O(N²) time complexity.</p>
    <div class="callout">
      <div class="callout-icon">💡</div>
      <p><b>Why 2 instead of 4?</b> Standard block matrix multiplication of this pattern would naively compute C₁ = A₁B₁ + A₂B₂ and C₂ = A₁B₂ + A₂B₁, requiring 4 recursive multiplications. By exploiting algebraic identities (similar to Strassen, but for this special pattern), we can reduce to just 2 multiplications using sum and difference matrices.</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Mathematical Foundation</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> THE ALGEBRAIC TRICK</h2>
    <h3>Standard Block Product</h3>
    <p>For C = A × B where A, B follow the special pattern:</p>
    <div class="formula">C₁ = A₁B₁ + A₂B₂
C₂ = A₁B₂ + A₂B₁</div>
    <p>This needs 4 multiplications. Instead, define:</p>
    <h3>The Reduction</h3>
    <div class="formula">P₁ = (A₁ + A₂) × (B₁ + B₂)
P₂ = (A₁ - A₂) × (B₁ - B₂)</div>
    <h3>Reconstruction (additions only)</h3>
    <div class="formula">C₁ = (P₁ + P₂) / 2
C₂ = (P₁ - P₂) / 2</div>
    <p>Only 2 recursive multiplications! The additions/subtractions cost O(N²) but that's the cheaper operation.</p>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> MASTER THEOREM PROOF</h2>
    <h3>Why does division by 2 work exactly?</h3>
    <p>Expanding P₁ + P₂ algebraically:</p>
    <div class="formula">P₁ + P₂ = (A₁+A₂)(B₁+B₂) + (A₁-A₂)(B₁-B₂)
       = 2A₁B₁ + 2A₂B₂  (always even!)</div>
    <p>So <code>(P₁+P₂)/2 = A₁B₁ + A₂B₂ = C₁</code>. Integer division is exact — no floating-point precision errors.</p>
    <h3>Master Theorem</h3>
    <div class="formula">T(n) = 2·T(n/2) + O(n²)</div>
    <p>a=2, b=2, f(n)=n². Critical exponent: log₂(2)=1. Since f(n)=n² = Ω(n^(1+1)) with ε=1, <b>Case 3 applies</b>. Regularity: 2·(n/2)² = n²/2 ≤ c·n² for c=1/2 &lt; 1.</p>
    <div class="formula">Time = Θ(n²)</div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// C Code Walkthrough</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> multiplySpecialRec() FUNCTION</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">void</span> multiplySpecialRec(A, lda, B, ldb, C, ldc, n, workspace) {
  <span class="k">if</span> (n == 1) { C[0] = A[0] * B[0]; <span class="k">return</span>; }

  <span class="k">int</span> k = n / 2;
  <span class="c">// Slice workspace into 6 named buffers</span>
  <span class="k">int</span>* S_A1 = workspace;           <span class="c">// (A1 + A2)</span>
  <span class="k">int</span>* S_B1 = S_A1 + k*k;         <span class="c">// (B1 + B2)</span>
  <span class="k">int</span>* P1   = S_B1 + k*k;         <span class="c">// result of recursive call 1</span>
  <span class="k">int</span>* S_A2 = P1   + k*k;         <span class="c">// (A1 - A2)</span>
  <span class="k">int</span>* S_B2 = S_A2 + k*k;         <span class="c">// (B1 - B2)</span>
  <span class="k">int</span>* P2   = S_B2 + k*k;         <span class="c">// result of recursive call 2</span>
  <span class="k">int</span>* next = P2 + k*k;

  <span class="c">// Zero-copy submatrix views</span>
  <span class="k">const int</span>* A1 = A; <span class="k">const int</span>* A2 = A + k;
  <span class="k">const int</span>* B1 = B; <span class="k">const int</span>* B2 = B + k;

  <span class="c">// Compute S_A1 = A1+A2, S_B1 = B1+B2 → O(n²)</span>
  <span class="k">for</span> (i=0;i&lt;k;i++) <span class="k">for</span> (j=0;j&lt;k;j++) {
    S_A1[i*k+j] = A1[i*lda+j] + A2[i*lda+j];
    S_B1[i*k+j] = B1[i*ldb+j] + B2[i*ldb+j];
  }
  multiplySpecialRec(S_A1,k,S_B1,k,P1,k,k,next); <span class="c">// call 1</span>

  <span class="c">// Compute S_A2 = A1-A2, S_B2 = B1-B2</span>
  <span class="k">for</span> (i=0;i&lt;k;i++) <span class="k">for</span> (j=0;j&lt;k;j++) {
    S_A2[i*k+j] = A1[i*lda+j] - A2[i*lda+j];
    S_B2[i*k+j] = B1[i*ldb+j] - B2[i*ldb+j];
  }
  multiplySpecialRec(S_A2,k,S_B2,k,P2,k,k,next); <span class="c">// call 2</span>

  <span class="c">// Reconstruct C — exact integer division by 2</span>
  <span class="k">for</span>(i=0;i&lt;k;i++) <span class="k">for</span>(j=0;j&lt;k;j++) {
    <span class="k">int</span> c1=(P1[idx]+P2[idx])/2;  <span class="c">// A1B1 + A2B2</span>
    <span class="k">int</span> c2=(P1[idx]-P2[idx])/2;  <span class="c">// A1B2 + A2B1</span>
    C11[i*ldc+j]=c1; C12[i*ldc+j]=c2;
    C21[i*ldc+j]=c2; C22[i*ldc+j]=c1;
  }
}</div>
    </div>
    <ul>
      <li><b>6 workspace buffers:</b> Two sum matrices (S_A1, S_B1), two difference matrices (S_A2, S_B2), two product matrices (P1, P2) — all sliced from a pre-allocated master workspace, no dynamic allocations inside recursion.</li>
      <li><b>Zero-copy views:</b> A1, A2, B1, B2 are just pointer offsets — no memory copying of sub-matrices.</li>
      <li><b>Exact reconstruction:</b> The mathematical guarantee that P1±P2 is always even means the division-by-2 is always precise for integer arithmetic.</li>
    </ul>
  </div>
</div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> WORKSPACE SIZE DERIVATION</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="c">// At level 1: 6 buffers of (n/2)² = 6n²/4</span>
<span class="c">// At level 2: 6 buffers of (n/4)² = 6n²/16</span>
<span class="c">// Sum (geometric series):</span>
<span class="c">// = 6n²/4 × 1/(1-1/4) = 6n²/4 × 4/3 = 2n²</span>
<span class="c">// Using 3n² gives 50% safety headroom</span>
size_t workspaceSize = 3 * (size_t)n * n;
<span class="k">int</span>* workspace = calloc(workspaceSize, <span class="k">sizeof</span>(<span class="k">int</span>));</div>
    </div>
    <p>The geometric series sum of all workspace needed at every recursion level converges to exactly 2n². We use 3n² to provide 50% safety headroom against off-by-one pointer arithmetic bugs.</p>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> GRAPH ANALYSIS</h2>
    <p>The <code>Matrix_Complexity_Analysis.png</code> shows three curves:</p>
    <ul>
      <li><b>Standard O(N³):</b> Shoots up steeply — cubic growth quickly dominates.</li>
      <li><b>Strassen O(N^2.807):</b> Less steep, but still super-quadratic.</li>
      <li><b>Special Pattern O(N²):</b> Nearly flat compared to the others — the quadratic curve hugs the bottom of the graph.</li>
    </ul>
    <div class="callout">
      <div class="callout-icon">⚠️</div>
      <p><b>Important:</b> The O(N²) complexity is only achievable because of the special pattern constraint. General matrices cannot be multiplied in O(N²) — the theoretical lower bound for general matrix multiplication is still an open problem in CS (≈ O(N^2.37)).</p>
    </div>
  </div>
</div>

</div>
</div>
