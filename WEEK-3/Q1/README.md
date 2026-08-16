<style>
@import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Rajdhani:wght@400;500;600;700&family=Fira+Code:wght@300;400;500&family=Inter:wght@300;400;500;600&display=swap');

* { box-sizing: border-box; }

.hud {
  background: #020408;
  background-image:
    radial-gradient(ellipse 80% 50% at 20% 20%, rgba(0,210,255,0.07) 0%, transparent 60%),
    radial-gradient(ellipse 60% 40% at 80% 80%, rgba(160,0,255,0.07) 0%, transparent 60%),
    linear-gradient(rgba(0,210,255,0.025) 1px, transparent 1px),
    linear-gradient(90deg, rgba(0,210,255,0.025) 1px, transparent 1px);
  background-size: auto, auto, 40px 40px, 40px 40px;
  color: #b0bec5;
  font-family: 'Inter', sans-serif;
  padding: 60px 50px;
  border-radius: 20px;
  position: relative;
  overflow: hidden;
  line-height: 1.75;
}

.hud::before {
  content: '';
  position: absolute; top: 0; left: 0; right: 0; height: 3px;
  background: linear-gradient(90deg, transparent, #00d2ff, #a000ff, #00d2ff, transparent);
  animation: border-run 4s linear infinite;
}
@keyframes border-run {
  0%   { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}

.hud-inner { position: relative; z-index: 1; }

/* ── HEADER ── */
.header { text-align: center; padding: 20px 0 40px; animation: fadeDown 0.8s ease both; }
.q-label {
  font-family: 'Fira Code', monospace;
  font-size: 0.9em;
  color: #00d2ff;
  letter-spacing: 6px;
  text-transform: uppercase;
  opacity: 0.8;
  margin-bottom: 8px;
}
.title {
  font-family: 'Orbitron', sans-serif;
  font-size: 3.4em;
  font-weight: 900;
  letter-spacing: 3px;
  background: linear-gradient(135deg, #00d2ff 0%, #ffffff 40%, #a000ff 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  margin: 0;
  line-height: 1.2;
  text-shadow: none;
  filter: drop-shadow(0 0 20px rgba(0,210,255,0.4));
}
.subtitle {
  font-family: 'Rajdhani', sans-serif;
  font-size: 1.3em;
  color: #ff2d87;
  letter-spacing: 5px;
  text-transform: uppercase;
  margin-top: 8px;
  font-weight: 600;
}

/* ── BADGES ── */
.badges { display: flex; flex-wrap: wrap; justify-content: center; gap: 12px; margin: 30px 0 50px; }
.badge {
  font-family: 'Fira Code', monospace;
  font-size: 0.85em;
  padding: 8px 20px;
  border-radius: 3px;
  border: 1px solid rgba(0,210,255,0.5);
  color: #00d2ff;
  background: rgba(0,210,255,0.05);
  letter-spacing: 2px;
  position: relative;
  overflow: hidden;
  transition: all 0.3s;
}
.badge:hover { background: rgba(0,210,255,0.15); transform: translateY(-2px); box-shadow: 0 0 20px rgba(0,210,255,0.3); }

/* ── 3D GRAPH FRAME ── */
.graph-stage { perspective: 1400px; display: flex; justify-content: center; margin: 50px 0; animation: fadeIn 1.2s ease both 0.3s; }
.graph-card {
  width: 88%; max-width: 820px;
  transform-style: preserve-3d;
  animation: levitate 9s ease-in-out infinite alternate;
  position: relative;
}
.graph-card img {
  width: 100%; border-radius: 10px;
  border: 1px solid rgba(160,0,255,0.4);
  box-shadow: 0 40px 80px rgba(0,0,0,0.9), 0 0 50px rgba(0,210,255,0.15);
  transform: translateZ(60px);
  display: block;
}
.graph-card::after {
  content: '';
  position: absolute; inset: -12px; border-radius: 20px;
  background: linear-gradient(135deg, rgba(0,210,255,0.3), rgba(160,0,255,0.3));
  transform: translateZ(-25px);
  filter: blur(30px);
  opacity: 0.5;
  animation: glow-pulse 4s ease-in-out infinite alternate;
}
@keyframes levitate {
  from { transform: rotateX(6deg) rotateY(-6deg) translateY(0px); }
  to   { transform: rotateX(-4deg) rotateY(6deg) translateY(-18px); }
}
@keyframes glow-pulse { from { opacity: 0.3; } to { opacity: 0.7; } }

/* ── SECTION DIVIDER ── */
.divider {
  display: flex; align-items: center; gap: 15px;
  margin: 50px 0 30px;
  animation: fadeIn 1s ease both;
}
.divider-line { flex: 1; height: 1px; background: linear-gradient(to right, transparent, rgba(0,210,255,0.4), transparent); }
.divider-label {
  font-family: 'Orbitron', sans-serif;
  font-size: 0.75em;
  color: rgba(0,210,255,0.6);
  letter-spacing: 4px;
  text-transform: uppercase;
  white-space: nowrap;
}

/* ── GRID ── */
.grid { display: grid; gap: 24px; margin-bottom: 24px; }
.grid-2 { grid-template-columns: repeat(2, 1fr); }
.grid-3 { grid-template-columns: repeat(3, 1fr); }
@media(max-width: 800px) { .grid-2, .grid-3 { grid-template-columns: 1fr; } }

/* ── PANELS ── */
.panel {
  background: rgba(8, 14, 24, 0.7);
  border: 1px solid rgba(255,255,255,0.05);
  border-radius: 14px;
  padding: 30px;
  position: relative;
  transition: transform 0.4s cubic-bezier(.23,1,.32,1), box-shadow 0.4s;
  animation: slideUp 0.8s ease both;
  backdrop-filter: blur(10px);
}
.panel:nth-child(1){animation-delay:0.1s}
.panel:nth-child(2){animation-delay:0.2s}
.panel:nth-child(3){animation-delay:0.3s}
.panel:nth-child(4){animation-delay:0.4s}

/* corner chrome */
.panel::before,.panel::after { content:''; position:absolute; width:16px; height:16px; transition:all 0.3s; }
.panel::before { top:0; left:0; border-top:2px solid #00d2ff; border-left:2px solid #00d2ff; border-radius:14px 0 0 0; }
.panel::after  { bottom:0; right:0; border-bottom:2px solid #a000ff; border-right:2px solid #a000ff; border-radius:0 0 14px 0; }
.panel:hover { transform: translateY(-8px); box-shadow: 0 24px 60px rgba(0,0,0,0.6), 0 0 30px rgba(0,210,255,0.08); }
.panel:hover::before { width:30px; height:30px; border-color:#ff2d87; }
.panel:hover::after  { width:30px; height:30px; border-color:#00d2ff; }

.panel.full { grid-column: 1 / -1; }
.panel.accent-cyan  { border-top: 2px solid rgba(0,210,255,0.5); }
.panel.accent-purple{ border-top: 2px solid rgba(160,0,255,0.5); }
.panel.accent-pink  { border-top: 2px solid rgba(255,45,135,0.5); }

/* ── PANEL TYPOGRAPHY ── */
.panel h2 {
  font-family: 'Orbitron', sans-serif;
  font-size: 1.15em;
  font-weight: 700;
  color: #ffffff;
  margin: 0 0 18px;
  letter-spacing: 1px;
  display: flex; align-items: center; gap: 10px;
}
.panel h2 .icon { color: #ff2d87; font-size: 0.7em; }
.panel h3 {
  font-family: 'Rajdhani', sans-serif;
  font-size: 1.2em;
  font-weight: 700;
  color: #00d2ff;
  margin: 22px 0 8px;
  letter-spacing: 2px;
  text-transform: uppercase;
  border-bottom: 1px dashed rgba(0,210,255,0.25);
  padding-bottom: 6px;
}
.panel p { font-size: 1.0em; color: #b0bec5; margin: 0 0 14px; }
.panel ul, .panel ol { padding-left: 20px; margin: 0 0 14px; }
.panel li { font-size: 0.98em; color: #b0bec5; margin-bottom: 8px; line-height: 1.7; }
.panel li b { color: #e0e8f0; }

/* ── CODE & TERMINAL ── */
code {
  font-family: 'Fira Code', monospace;
  font-size: 0.87em;
  background: rgba(0,0,0,0.6);
  color: #00d2ff;
  padding: 2px 7px;
  border-radius: 4px;
  border: 1px solid rgba(0,210,255,0.2);
}

.term {
  background: #000;
  border-radius: 8px;
  overflow: hidden;
  margin: 16px 0;
  border: 1px solid rgba(255,255,255,0.08);
  box-shadow: inset 0 0 20px rgba(0,0,0,0.5);
}
.term-bar {
  background: #1a1a1a;
  padding: 8px 14px;
  display: flex; align-items: center; gap: 7px;
  border-bottom: 1px solid rgba(255,255,255,0.06);
}
.term-dot { width:11px; height:11px; border-radius:50%; }
.term-dot.r { background:#ff5f57; } .term-dot.y { background:#febc2e; } .term-dot.g { background:#28c840; }
.term-body {
  padding: 16px 20px;
  font-family: 'Fira Code', monospace;
  font-size: 0.92em;
  line-height: 1.7;
  color: #00ff88;
  white-space: pre;
}
.term-body .c { color: #666; }
.term-body .k { color: #00d2ff; }
.term-body .s { color: #ff2d87; }

/* ── FORMULA BLOCK ── */
.formula {
  background: linear-gradient(135deg, rgba(0,0,0,0.8), rgba(10,5,20,0.8));
  border: 1px solid rgba(160,0,255,0.3);
  border-radius: 8px;
  padding: 16px 24px;
  text-align: center;
  font-family: 'Fira Code', monospace;
  font-size: 1.05em;
  color: #d0a0ff;
  letter-spacing: 1.5px;
  margin: 14px 0;
  box-shadow: 0 0 20px rgba(160,0,255,0.1);
}

/* ── COMPARISON TABLE ── */
.compare-table {
  width: 100%; border-collapse: collapse; font-family: 'Fira Code', monospace; font-size: 0.88em;
}
.compare-table th {
  background: rgba(0,210,255,0.08);
  color: #00d2ff;
  padding: 10px 16px;
  text-align: left;
  border-bottom: 2px solid rgba(0,210,255,0.2);
  font-weight: 600;
  letter-spacing: 1px;
}
.compare-table td {
  padding: 10px 16px;
  color: #b0bec5;
  border-bottom: 1px solid rgba(255,255,255,0.04);
}
.compare-table tr:hover td { background: rgba(255,255,255,0.02); }
.compare-table .winner { color: #00ff88; font-weight: 600; }
.compare-table .loser  { color: #ff4060; }

/* ── CALLOUT ── */
.callout {
  display: flex; gap: 14px; align-items: flex-start;
  background: rgba(255,45,135,0.05);
  border: 1px solid rgba(255,45,135,0.2);
  border-radius: 8px;
  padding: 16px 20px;
  margin: 16px 0;
}
.callout-icon { font-size: 1.3em; line-height: 1; flex-shrink: 0; margin-top: 2px; }
.callout p { color: #e0cce8; margin: 0; font-size: 0.97em; }

/* ── STEP FLOW ── */
.steps { counter-reset: step; list-style: none; padding: 0; margin: 0; }
.steps li {
  counter-increment: step;
  position: relative;
  padding: 14px 14px 14px 56px;
  border-left: 2px solid rgba(0,210,255,0.15);
  margin-bottom: 0;
}
.steps li::before {
  content: counter(step);
  position: absolute; left: -16px; top: 14px;
  width: 30px; height: 30px;
  background: rgba(0,210,255,0.15);
  border: 1px solid rgba(0,210,255,0.4);
  border-radius: 50%;
  color: #00d2ff;
  font-family: 'Orbitron', sans-serif;
  font-size: 0.75em;
  display: flex; align-items: center; justify-content: center;
  font-weight: 700;
}
.steps li:last-child { border-color: rgba(160,0,255,0.3); }
.steps li:last-child::before { background: rgba(160,0,255,0.15); border-color: rgba(160,0,255,0.4); color: #a000ff; }

/* ── KEYFRAMES ── */
@keyframes fadeDown { from { opacity:0; transform:translateY(-25px); } to { opacity:1; transform:none; } }
@keyframes fadeIn   { from { opacity:0; transform:scale(0.95); } to { opacity:1; transform:scale(1); } }
@keyframes slideUp  { from { opacity:0; transform:translateY(35px); } to { opacity:1; transform:none; } }
</style>

<div class="hud">
<div class="hud-inner">

<!-- ═══ HEADER ═══ -->
<div class="header">
  <div class="q-label">// Week-3 · Question 01 · DAA Lab</div>
  <h1 class="title">BINARY vs TERNARY SEARCH</h1>
  <div class="subtitle">Divide & Conquer · Comparison Analysis</div>
</div>

<div class="badges">
  <span class="badge">TIME: O(log N)</span>
  <span class="badge">SPACE: O(1)</span>
  <span class="badge">LANG: C</span>
  <span class="badge">PARADIGM: D&C</span>
  <span class="badge">WEEK: 03</span>
</div>

<!-- ═══ GRAPH ═══ -->
<div class="graph-stage">
  <div class="graph-card">
    <img src="./full_complexity_analysis.png" alt="Binary vs Ternary Complexity Graph">
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<!-- ═══ QUESTION ═══ -->
<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Compare <b>Binary Search</b> and <b>Ternary Search</b> on a sorted array to determine which algorithm requires fewer key comparisons in the worst case. Implement both in C, count the exact number of comparisons made, and prove your finding through both empirical testing and theoretical (recurrence relation) analysis.</p>
    <div class="callout">
      <div class="callout-icon">⚠️</div>
      <p><b>Core Intuition to Prove:</b> Though Ternary Search divides into 3 parts (reducing depth), it needs more comparisons <em>per level</em> to decide which third to enter. Does saving depth offset the extra per-level cost?</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Algorithm Deep Dive</div><div class="divider-line"></div></div>

<!-- ═══ ALGORITHM ANALYSIS ═══ -->
<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> BINARY SEARCH</h2>
    <h3>Working Principle</h3>
    <p>Computes a single midpoint <code>mid</code> splitting the search space into 2 halves. At each step, performs 2 comparisons:</p>
    <ol class="steps">
      <li><b>Equality check:</b> <code>arr[mid] == target</code> → Found! Return.</li>
      <li><b>Direction check:</b> <code>arr[mid] > target</code> → go left; else go right.</li>
    </ol>
    <h3>Recurrence Relation</h3>
    <div class="formula">T(n) = T(n/2) + 2</div>
    <p>Each level: 2 comparisons, problem size halved. Tree height = <code>log₂(n)</code>.</p>
    <div class="formula">C<sub>binary</sub>(n) = 2 · log₂(n)</div>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> TERNARY SEARCH</h2>
    <h3>Working Principle</h3>
    <p>Computes two midpoints <code>mid1</code> and <code>mid2</code>, splitting the space into 3 thirds. At each step, performs up to 4 comparisons:</p>
    <ol class="steps">
      <li><b>Check mid1:</b> <code>arr[mid1] == target</code> → Found!</li>
      <li><b>Check mid2:</b> <code>arr[mid2] == target</code> → Found!</li>
      <li><b>Compare target vs mid1:</b> <code>target < arr[mid1]</code> → left third.</li>
      <li><b>Compare target vs mid2:</b> <code>target > arr[mid2]</code> → right third; else middle.</li>
    </ol>
    <h3>Recurrence Relation</h3>
    <div class="formula">T(n) = T(n/3) + 4</div>
    <p>Tree height = <code>log₃(n) = log₂(n)/log₂(3) ≈ log₂(n)/1.585</code></p>
    <div class="formula">C<sub>ternary</sub>(n) = 4 · log₃(n) ≈ 2.524 · log₂(n)</div>
  </div>
</div>

<!-- comparison table -->
<div class="panel full">
  <h2><span class="icon">▶</span> SIDE-BY-SIDE COMPARISON</h2>
  <table class="compare-table">
    <tr>
      <th>Metric</th>
      <th>Binary Search</th>
      <th>Ternary Search</th>
    </tr>
    <tr>
      <td>Partitions per step</td>
      <td class="winner">2</td>
      <td>3</td>
    </tr>
    <tr>
      <td>Comparisons per level (worst)</td>
      <td class="winner">2</td>
      <td class="loser">4</td>
    </tr>
    <tr>
      <td>Tree height</td>
      <td>log₂(n)</td>
      <td class="winner">log₃(n) [smaller]</td>
    </tr>
    <tr>
      <td>Total worst-case comparisons</td>
      <td class="winner">2·log₂(n)</td>
      <td class="loser">2.524·log₂(n)</td>
    </tr>
    <tr>
      <td>Overhead vs Binary</td>
      <td class="winner">Baseline</td>
      <td class="loser">+26% comparisons</td>
    </tr>
    <tr>
      <td>Time Complexity</td>
      <td>O(log n)</td>
      <td>O(log n)</td>
    </tr>
    <tr>
      <td>Space Complexity</td>
      <td>O(1)</td>
      <td>O(1)</td>
    </tr>
    <tr>
      <td><b>Winner</b></td>
      <td class="winner">✔ BINARY SEARCH</td>
      <td></td>
    </tr>
  </table>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// C Code Walkthrough</div><div class="divider-line"></div></div>

<!-- ═══ C CODE EXPLANATION ═══ -->
<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> binarySearch() FUNCTION</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">int</span> binarySearch(arr, size, target, *comparisons) {
  <span class="k">int</span> low = 1, high = size;
  <span class="k">while</span> (low <= high) {
    <span class="k">int</span> mid = low + (high - low) / 2;
    (*comparisons)++;           <span class="c">// equality check</span>
    <span class="k">if</span> (arr[mid] == target) <span class="k">return</span> mid;
    (*comparisons)++;           <span class="c">// direction check</span>
    <span class="k">if</span> (arr[mid] > target) high = mid - 1;
    <span class="k">else</span> low = mid + 1;
  }
  <span class="k">return</span> -1;
}</div>
    </div>
    <ul>
      <li><b>1-based indexing:</b> <code>low = 1, high = size</code> — matches mathematical convention.</li>
      <li><b>Overflow-safe mid:</b> Uses <code>low + (high-low)/2</code> instead of <code>(low+high)/2</code> to prevent integer overflow for large arrays.</li>
      <li><b>Per-iteration cost:</b> Worst case 2 comparisons — one equality, one direction.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> ternarySearch() FUNCTION</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">int</span> ternarySearch(arr, size, target, *comparisons) {
  <span class="k">int</span> low=1, high=size;
  <span class="k">while</span> (low <= high) {
    <span class="k">int</span> mid1 = low + (high-low)/3;
    <span class="k">int</span> mid2 = high - (high-low)/3;
    (*comparisons)++;  <span class="k">if</span>(arr[mid1]==target) <span class="k">return</span> mid1;
    (*comparisons)++;  <span class="k">if</span>(arr[mid2]==target) <span class="k">return</span> mid2;
    (*comparisons)++;
    <span class="k">if</span> (target < arr[mid1])  high = mid1-1;
    <span class="k">else</span> {
      (*comparisons)++;
      <span class="k">if</span> (target > arr[mid2]) low = mid2+1;
      <span class="k">else</span> { low=mid1+1; high=mid2-1; }
    }
  }
  <span class="k">return</span> -1;
}</div>
    </div>
    <ul>
      <li><b>Two midpoints:</b> <code>mid1 = low + range/3</code> and <code>mid2 = high - range/3</code>.</li>
      <li><b>4 comparison path:</b> Equality at mid1 → equality at mid2 → direction vs mid1 → direction vs mid2.</li>
      <li><b>3 zones:</b> Left (< mid1), Middle (between mid1 and mid2), Right (> mid2).</li>
    </ul>
  </div>
</div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> MAIN FUNCTION: MEMORY & I/O ARCHITECTURE</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="c">// 1-based indexing trick: allocate n+1 slots, ignore index 0</span>
<span class="k">int</span> *arr = (<span class="k">int</span> *)malloc((n + 1) * <span class="k">sizeof</span>(<span class="k">int</span>));

<span class="c">// Sorted input validation — detect unsorted input early</span>
<span class="k">if</span> (i > 1 && arr[i] < arr[i-1])
    printf("[!] WARNING: Input not sorted. Results may be inaccurate.\n");

<span class="c">// Run both searches on same data</span>
<span class="k">int</span> bin_comp = 0, tern_comp = 0;
binarySearch (arr, n, target, &bin_comp);
ternarySearch(arr, n, target, &tern_comp);

<span class="c">// Compare results: empirical validation of theoretical proof</span>
printf("Binary  comparisons: %d\n", bin_comp);
printf("Ternary comparisons: %d\n", tern_comp);</div>
    </div>
    <h3>Key Design Decisions</h3>
    <ul>
      <li><b>Dynamic allocation of (n+1):</b> The extra slot at index 0 is intentionally unused, enabling clean 1-based array indexing for cleaner math without pointer arithmetic adjustments.</li>
      <li><b>Comparison counters by pointer:</b> <code>int *comparisons</code> is passed to each function, letting the main function receive the count as an output parameter without global variables.</li>
      <li><b>Sorted-order warning:</b> Both algorithms silently produce wrong answers on unsorted input. The runtime check at each element alerts the user before damage is done.</li>
      <li><b>Memory cleanup:</b> <code>free(arr)</code> is called before exit to prevent memory leaks.</li>
    </ul>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Graph Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> WHAT THE GRAPH SHOWS</h2>
    <p>The <code>full_complexity_analysis.png</code> graph plots <b>number of comparisons vs array size N</b> for both algorithms across a range of values (typically N = 100 to 10,000+).</p>
    <ul>
      <li><b>Both curves flatten logarithmically</b> — confirming the <code>O(log N)</code> bound visually.</li>
      <li><b>Ternary curve consistently sits ABOVE Binary</b> — by approximately 26% at every data point tested.</li>
      <li><b>The gap widens slightly with N</b> — because the constant factor multiplies a slightly larger log value each time.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> MATHEMATICAL CONFIRMATION</h2>
    <h3>Formal Derivation</h3>
    <div class="formula">C_ternary / C_binary = (4/log₂3) / 2 = 2/log₂3 ≈ 1.26</div>
    <p>This <b>1.26× overhead</b> = <b>26% MORE comparisons</b> for Ternary in all worst-case scenarios.</p>
    <div class="callout">
      <div class="callout-icon">✅</div>
      <p><b>Conclusion:</b> Despite having a shallower tree (log₃n < log₂n), the Ternary Search's 4-comparison-per-level cost outweighs the depth savings. Binary Search is strictly more efficient for sorted array search.</p>
    </div>
  </div>
</div>

</div>
</div>
