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
  <div class="q-label">// Week-3 · Question 02 · DAA Lab</div>
  <h1 class="title">DEFECTIVE COIN SEARCH</h1>
  <div class="subtitle">Balance Scale · Divide & Conquer · O(log N) Weighings</div>
</div>

<div class="badges">
  <span class="badge">WEIGHINGS: O(log N)</span>
  <span class="badge">CPU TIME: O(N log N)</span>
  <span class="badge">SPACE: O(1)</span>
  <span class="badge">LANG: C</span>
  <span class="badge">ITERATIVE D&C</span>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Given <b>N coins</b> that all look identical, exactly one coin is defective — it is slightly lighter than all genuine coins. Using a balance scale that can only tell you <em>which side is heavier</em> (or if they balance), find the defective coin using at most <code>⌈log₂(N)⌉ + c</code> weighing operations.</p>
    <p>Implement this in C using the Divide and Conquer approach. Simulate the physical balance scale programmatically. Also derive the time complexity via the Master Theorem and prove why binary halving optimally solves this.</p>
    <div class="callout">
      <div class="callout-icon">💡</div>
      <p><b>Key Insight:</b> A balance scale gives a 3-way result — Left lighter, Right lighter, or Equal. Divide the coins into two equal groups. The imbalanced side must contain the defective coin. Repeat on that half. Each weighing eliminates 50% of suspects — exponential elimination!</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Algorithm Deep Dive</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> ALGORITHM STEPS</h2>
    <ol class="steps">
      <li><b>Split:</b> Divide current coin range [start, end] into two equal halves of size <code>floor(count/2)</code>. If count is odd, one coin is set aside temporarily.</li>
      <li><b>Weigh:</b> Call <code>weighScale()</code> to compare the total weight of the left half vs. the right half.</li>
      <li><b>Decision Branch:</b> If left is lighter → defective coin is in left half. If right is lighter → defective coin is in right half. If balanced → check the set-aside coin.</li>
      <li><b>Recurse:</b> Narrow the active range to the half containing the suspect and repeat from Step 1 until only 1 coin remains.</li>
    </ol>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> COMPLEXITY ANALYSIS</h2>
    <h3>Physical Weighings</h3>
    <p>Each step performs exactly 1 weighing and halves the problem:</p>
    <div class="formula">W(n) = W(n/2) + 1</div>
    <p>Master Theorem: a=1, b=2, f(n)=O(1). Since n^(log₂1)=1 = Θ(f(n)) → Case 2.</p>
    <div class="formula">W(n) = Θ(log₂ n) ✅</div>
    <h3>CPU Computational Time</h3>
    <p>Each <code>weighScale()</code> call iterates over n/2 elements to compute sums:</p>
    <div class="formula">T(n) = T(n/2) + O(n)  →  O(n log n)</div>
  </div>
</div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> ODD vs EVEN CASE HANDLING</h2>
    <div class="grid grid-2">
      <div>
        <h3>EVEN COUNT</h3>
        <p>Both halves are equal size. The balance scale result is definitive:</p>
        <ul>
          <li>Scale tips → defective coin is in the lighter half (guaranteed).</li>
          <li>Scale balances → <em>all</em> coins in the current range are perfect. No defective coin exists in this batch. Return <code>-1</code>.</li>
        </ul>
      </div>
      <div>
        <h3>ODD COUNT</h3>
        <p>The last coin cannot be placed on either side equally. Protocol:</p>
        <ul>
          <li>Set the final coin aside as the "suspect".</li>
          <li>Weigh the two equal halves.</li>
          <li>If balanced → the set-aside coin is the suspect. Perform a final verification weighing against a known-good coin.</li>
          <li>If not balanced → proceed normally; the set-aside coin is innocent.</li>
        </ul>
      </div>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// C Code Walkthrough</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> weighScale() — THE ORACLE</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">int</span> weighScale(coins, lStart, lEnd, rStart, rEnd) {
  <span class="k">long long</span> weightLeft = 0, weightRight = 0;

  <span class="k">for</span>(i = lStart; i < lEnd; i++) weightLeft  += coins[i];
  <span class="k">for</span>(i = rStart; i < rEnd; i++) weightRight += coins[i];

  <span class="k">if</span> (weightLeft  < weightRight) <span class="k">return</span> -1; <span class="c">// left lighter</span>
  <span class="k">if</span> (weightRight < weightLeft)  <span class="k">return</span>  1; <span class="c">// right lighter</span>
  <span class="k">return</span> 0;                                   <span class="c">// balanced</span>
}</div>
    </div>
    <ul>
      <li><b>long long accumulators:</b> Prevents overflow when summing hundreds of coin weights. Regular <code>int</code> would overflow for large N.</li>
      <li><b>Returns -1/0/+1:</b> Three-state output matches the physical scale's three possible outcomes.</li>
      <li><b>Half-open ranges:</b> Uses <code>[start, end)</code> convention — cleaner loop bounds and no off-by-one errors.</li>
    </ul>
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> findDefectiveCoin() — MAIN LOGIC</h2>
    <div class="term">
      <div class="term-bar"><div class="term-dot r"></div><div class="term-dot y"></div><div class="term-dot g"></div></div>
      <div class="term-body"><span class="k">int</span> start=0, end=n;
<span class="k">bool</span> defectGuaranteed = false;

<span class="k">while</span>((end - start) > 1) {
  <span class="k">int</span> count = end - start;
  <span class="k">int</span> mid   = count / 2;
  <span class="k">int</span> result = weighScale(coins, start,
                 start+mid, start+mid, start+mid*2);

  <span class="k">if</span> (result == -1) { end = start + mid; defectGuaranteed = true; }
  <span class="k">else if</span> (result == 1) { start += mid;   defectGuaranteed = true; }
  <span class="k">else</span> {
    <span class="k">if</span> (count % 2 == 0) <span class="k">return</span> -1; <span class="c">// even, balanced = no defect</span>
    <span class="k">else</span> {
      <span class="c">// odd: check set-aside coin vs known-good</span>
      <span class="k">int</span> extraIdx = end - 1;
      <span class="k">if</span>(weighScale(coins, extraIdx, extraIdx+1,
                    start, start+1) == -1) <span class="k">return</span> extraIdx;
      <span class="k">return</span> -1;
    }
  }
}
<span class="k">if</span>((end-start)==1 && defectGuaranteed) <span class="k">return</span> start;
<span class="k">return</span> -1;</div>
    </div>
    <ul>
      <li><b>Iterative D&C:</b> Standard recursion replaced with a <code>while</code> loop — eliminates <code>O(log n)</code> call stack frames.</li>
      <li><b>defectGuaranteed flag:</b> Tracks whether a defect has been confirmed by a scale imbalance. Prevents falsely returning a coin when the algorithm narrows to 1 without ever seeing an imbalance.</li>
    </ul>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Why This is Optimal</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> INFORMATION-THEORETIC LOWER BOUND</h2>
    <p>Each weighing yields one of 3 outcomes. After <code>k</code> weighings, the maximum number of distinguishable states is <code>3^k</code>. To identify 1 defective coin among N:</p>
    <div class="formula">3^k ≥ N  →  k ≥ log₃(N)  →  k = ⌈log₃(N)⌉</div>
    <p>However, since the defect is known to make the coin lighter (unidirectional), the balance gives effectively binary information per step, giving us <code>⌈log₂(N)⌉</code> as the minimum weighings required. Our algorithm achieves this — it is <b>provably optimal</b>.</p>
  </div>
</div>

</div>
</div>
