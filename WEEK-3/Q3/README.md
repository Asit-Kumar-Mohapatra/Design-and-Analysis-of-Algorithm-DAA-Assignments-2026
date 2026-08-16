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
.compare-table{width:100%;border-collapse:collapse;font-family:'Fira Code',monospace;font-size:0.88em;}
.compare-table th{background:rgba(0,210,255,0.08);color:#00d2ff;padding:10px 16px;text-align:left;border-bottom:2px solid rgba(0,210,255,0.2);font-weight:600;letter-spacing:1px;}
.compare-table td{padding:10px 16px;color:#b0bec5;border-bottom:1px solid rgba(255,255,255,0.04);}
.compare-table tr:hover td{background:rgba(255,255,255,0.02);}
.compare-table .winner{color:#00ff88;font-weight:600;}
</style>

<div class="hud">
<div class="hud-inner">

<div class="header">
  <div class="q-label">// Week-3 · Question 03 · DAA Lab</div>
  <h1 class="title">MIN & MAX FINDER</h1>
  <div class="subtitle">Tournament Pair-Peeling · D&C · ⌈3N/2⌉ - 2 Comparisons</div>
</div>

<div class="badges">
  <span class="badge">TIME: O(N)</span>
  <span class="badge">COMPARISONS: ⌈3N/2⌉ - 2</span>
  <span class="badge">SPACE: O(N/2)</span>
  <span class="badge">LANG: C</span>
  <span class="badge">RECURSIVE D&C</span>
</div>

<div class="graph-stage">
  <div class="graph-card">
    <img src="./max_min_comparison_graph.png" alt="Max Min Comparison Graph">
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Given an unsorted array of <b>N integers</b>, find both the <b>minimum</b> and <b>maximum</b> elements simultaneously using the Divide and Conquer approach. The goal is to minimize the total number of key comparisons. Prove that your implementation achieves exactly <code>⌈3N/2⌉ - 2</code> comparisons, which is the theoretical minimum for this problem.</p>
    <div class="callout">
      <div class="callout-icon">💡</div>
      <p><b>Why not two separate passes?</b> Finding max alone requires N-1 comparisons, and finding min alone requires N-1 comparisons, for a total of 2N-2. The D&C pair-peeling strategy achieves only ⌈3N/2⌉ - 2 comparisons — nearly 25% fewer operations!</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Algorithm Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> NAIVE vs D&C</h2>
    <h3>Naive Two-Pass Approach</h3>
    <ul>
      <li>Pass 1: Scan all N elements for max → <b>N-1 comparisons</b></li>
      <li>Pass 2: Scan all N elements for min → <b>N-1 comparisons</b></li>
      <li><b>Total: 2N - 2 comparisons</b></li>
    </ul>
    <h3>Divide & Conquer Tournament</h3>
    <ul>
      <li>Extract front pair, compare internally → <b>1 comparison</b></li>
      <li>Recurse on remaining N-2 elements</li>
      <li>Merge: compare pairMax vs globalMax AND pairMin vs globalMin → <b>2 comparisons</b></li>
      <li><b>Total: ⌈3N/2⌉ - 2 comparisons</b></li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> RECURRENCE & PROOF</h2>
    <h3>Recurrence Relation</h3>
    <div class="formula">T(1) = 0  (base: single element, 0 cmps)
T(2) = 1  (base: compare pair, 1 cmp)
T(N) = T(N-2) + 3</div>
    <h3>Solving the Recurrence</h3>
    <p>Unrolling: <code>T(N) = 3*(N/2-1) + 1 = 3N/2 - 2</code></p>
    <div class="formula">T(N) = ⌈3N/2⌉ - 2</div>
    <p>For example: N=8 → T(8) = 12-2 = <b>10 comparisons</b> (vs 14 naive)</p>
    <h3>Is this optimal?</h3>
    <p>Yes. Information-theoretic lower bound proves <code>⌈3N/2⌉ - 2</code> is the minimum possible comparisons to find both min and max simultaneously.</p>
  </div>
</div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> HOW THE PAIR PEELING WORKS</h2>
    <ol class="steps">
      <li><b>Extract front pair:</b> Take <code>arr[low]</code> and <code>arr[low+1]</code> from the array front.</li>
      <li><b>Order the pair (1 comparison):</b> <code>if (pairA > pairB)</code> → set pairMax=A, pairMin=B; else pairMax=B, pairMin=A.</li>
      <li><b>Recurse deeper:</b> Call <code>getMinMaxDNC(arr, low+2, high, &restMin, &restMax)</code> — recurse on everything <em>except</em> this pair.</li>
      <li><b>Merge max (1 comparison):</b> <code>*max = (pairMax > restMax) ? pairMax : restMax</code></li>
      <li><b>Merge min (1 comparison):</b> <code>*min = (pairMin &lt; restMin) ? pairMin : restMin</code></li>
    </ol>
    <p style="margin-top:14px;">Each "peel" costs exactly 3 comparisons and handles 2 elements. For N elements, we peel N/2 times = 3*(N/2) total, minus 2 for the base cases = <b>⌈3N/2⌉ - 2</b>.</p>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// C Code Walkthrough</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> getMinMaxDNC() FUNCTION</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">void</span> getMinMaxDNC(arr, low, high, *min, *max, *cmp) {
  <span class="k">int</span> n = high - low + 1;

  <span class="k">if</span> (n == 1) { *min = *max = arr[low]; <span class="k">return</span>; }

  <span class="k">if</span> (n == 2) {            <span class="c">// base case: 1 comparison</span>
    (*cmp)++;
    <span class="k">if</span> (arr[low] > arr[high]) { *max=arr[low]; *min=arr[high]; }
    <span class="k">else</span>                      { *max=arr[high];*min=arr[low];  }
    <span class="k">return</span>;
  }

  <span class="c">// Peel the front pair — 1 comparison</span>
  <span class="k">int</span> pA=arr[low], pB=arr[low+1], pMax, pMin;
  (*cmp)++;
  <span class="k">if</span>(pA > pB){ pMax=pA; pMin=pB; } <span class="k">else</span> { pMax=pB; pMin=pA; }

  <span class="k">int</span> rMin, rMax;
  getMinMaxDNC(arr, low+2, high, &rMin, &rMax, cmp);

  (*cmp)++; *max = (pMax > rMax) ? pMax : rMax; <span class="c">// merge</span>
  (*cmp)++; *min = (pMin < rMin) ? pMin : rMin; <span class="c">// merge</span>
}</div>
    </div>
    <ul>
      <li><b>Three base cases:</b> n=1 (0 cmp, trivial), n=2 (1 cmp, order pair), n≥3 (peel + recurse + merge).</li>
      <li><b>Exact counter:</b> <code>*cmp</code> tracks every real comparison, enabling live verification against the theoretical bound.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> RUNTIME VALIDATION SYSTEM</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">void</span> printAnalysis(<span class="k">int</span> n, <span class="k">long long</span> actual) {
  <span class="c">// Compute theoretical bound: ceil(3n/2) - 2</span>
  <span class="k">long long</span> theory = (3LL * n + 1) / 2 - 2;
  <span class="k">if</span> (theory < 0) theory = 0;

  printf("Actual Comparisons  : %lld\n", actual);
  printf("Theoretical Bound   : %lld\n", theory);

  <span class="k">if</span> (actual <= theory)
    printf("Status: SUCCESS ✅ (matches ceil(3n/2)-2)\n");
  <span class="k">else</span>
    printf("Status: FAILED ❌ (exceeded bound)\n");
}</div>
    </div>
    <ul>
      <li><b>Formula:</b> <code>(3LL * n + 1) / 2 - 2</code> computes <code>⌈3n/2⌉ - 2</code> using integer arithmetic without floating point.</li>
      <li><b>Live proof:</b> Every execution self-validates against the mathematical bound, acting as a built-in correctness test.</li>
    </ul>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Graph Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> GRAPH INTERPRETATION</h2>
    <p>The <code>max_min_comparison_graph.png</code> plots <b>comparison count vs N</b> for both approaches.</p>
    <ul>
      <li><b>Two linear lines:</b> Both approaches are O(N), so both appear as straight lines — not curves.</li>
      <li><b>Constant gap:</b> D&C line runs strictly below the Naive line, with the gap growing proportionally with N.</li>
      <li><b>At N=100:</b> Naive = 198 cmp; D&C = 148 cmp. Savings = 50 comparisons (25.25% reduction).</li>
      <li><b>At N=1000:</b> Naive = 1998; D&C = 1498. Savings = 500 comparisons.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> REAL-WORLD SIGNIFICANCE</h2>
    <div class="callout">
      <div class="callout-icon">🚀</div>
      <p>In embedded systems, sensor arrays, and streaming pipelines where comparisons are expensive hardware operations (e.g. comparing ADC readings on DSPs), this 25% reduction directly translates to power savings and throughput improvements.</p>
    </div>
    <p>The key takeaway: <b>asymptotic complexity (O(N)) can be identical between two algorithms, but constant factors can still differ significantly</b> — making this a demonstration that Big-O notation alone doesn't tell the full performance story.</p>
  </div>
</div>

</div>
</div>
