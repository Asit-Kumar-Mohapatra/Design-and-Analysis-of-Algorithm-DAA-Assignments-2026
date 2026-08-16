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
  <div class="q-label">// Week-3 · Question 06 · DAA Lab</div>
  <h1 class="title">SELECTION SORT</h1>
  <div class="subtitle">Loop Invariant Proof · Mathematical Correctness Validation</div>
</div>

<div class="badges">
  <span class="badge">TIME: Θ(N²)</span>
  <span class="badge">SPACE: O(1) IN-PLACE</span>
  <span class="badge">COMPARISONS: N(N-1)/2</span>
  <span class="badge">LANG: C</span>
  <span class="badge">LOOP INVARIANTS</span>
</div>

<div class="graph-stage">
  <div class="graph-card">
    <img src="./Selection_Sort_Complexity_Analysis.png" alt="Selection Sort Complexity">
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Problem Statement</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> THE QUESTION</h2>
    <p>Implement the <b>Selection Sort</b> algorithm in C and formally prove its correctness using <b>Loop Invariants</b> — a mathematical technique used to verify algorithm correctness. A loop invariant is a logical condition that:</p>
    <ul>
      <li>Is true <b>before</b> the loop starts (Initialization)</li>
      <li>Remains true <b>after every iteration</b> (Maintenance)</li>
      <li>Implies the algorithm's <b>correctness at termination</b> (Termination)</li>
    </ul>
    <p>Also derive the exact comparison count, time complexity, and space complexity. Explain why the outer loop only runs <code>n-1</code> times (not n times).</p>
    <div class="callout">
      <div class="callout-icon">💡</div>
      <p><b>Selection Sort Key Idea:</b> In each iteration <code>i</code>, scan the entire remaining unsorted sub-array to find the minimum element, then place it at position <code>i</code>. After <code>n-1</code> iterations, the array is fully sorted.</p>
    </div>
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// The Formal Loop Invariant Proof</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-pink">
    <h2><span class="icon">▶</span> COMPLETE LOOP INVARIANT PROOF</h2>

    A loop invariant is a property about the state of the algorithm that must be true before the loop starts, must remain true after every iteration, and must lead us to conclude that the algorithm is correct when the loop ends. For Selection Sort, the invariant is: at the beginning of each pass through the outer loop, the left portion of the array already contains exactly the correct smallest elements arranged in sorted order. Whatever has been placed on the left side is final and will not change again.

    Initialization — Before the very first pass begins, no elements have been processed yet, so the sorted portion is completely empty. An empty collection trivially satisfies any ordering condition, because there are no elements to be out of order. The invariant is vacuously true at the start. This is the base case of the proof and it holds without question.

    Maintenance — Assume the invariant is true at the start of some arbitrary pass. At that point, the left side holds correctly sorted smallest elements and the right side contains everything else. During this pass, the algorithm scans the entire unsorted right portion and identifies the smallest element within it. That element is smaller than everything still unsorted, and since the sorted portion already holds all elements smaller than it, this newly found minimum is the next element in overall sorted order. It is then moved to the boundary position, extending the sorted region by exactly one. After the pass completes, the sorted portion is one element larger and still correctly ordered. The invariant is preserved.

    Termination — The outer loop runs until all but the last element have been placed. When the loop ends, the sorted region covers every element except the very last one. Since all smaller elements have already been placed correctly, the final remaining element must be the largest in the array. It therefore sits in its correct position automatically, without needing any additional work. At this point the entire array is sorted from smallest to largest. The invariant combined with the loop ending directly proves correctness.

    Why n minus one passes are sufficient — After n minus one passes, every element except the last has been explicitly placed in its correct sorted position. The last element has no other place to go, because all smaller values are already to its left, so it must already be in the right position. Performing one more pass would accomplish nothing. The algorithm would scan a single-element region, compare it against itself, find no improvement, and make no change. This extra work would be entirely redundant. Stopping at n minus one passes is both correct and optimal.
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Complexity Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> EXACT COMPARISON COUNT</h2>
    In each pass i, the inner scan checks every element from position i+1 to the end of the array, making exactly n minus 1 minus i comparisons. Summing this over all passes from i equals 0 to i equals n minus 2 gives the total as the sum of the series (n-1) + (n-2) + ... + 2 + 1, which equals n(n-1)/2.

    As a concrete example: for an array of 10 elements this is 45 comparisons; for 100 elements it is 4,950 comparisons; for 1,000 elements it is 499,500 comparisons. The count grows as the square of n.

    The most important characteristic of Selection Sort is that it has no early termination. Unlike Insertion Sort or Bubble Sort, it always performs exactly n(n-1)/2 comparisons regardless of whether the input is already sorted, randomly ordered, or in reverse. This is why the best case, average case, and worst case are all identical and equal to Theta of n squared.
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> SPACE COMPLEXITY: O(1)</h2>
    Selection Sort sorts entirely in-place, meaning it rearranges elements within the original array without ever allocating any additional storage that grows with the input size. The only extra memory used consists of four simple integer variables: one to track the current outer boundary, one to drive the inner scan, one to remember the position of the current minimum, and one temporary variable used during the swap step. None of these grow larger or multiply in count as the array size increases.

    Because the amount of extra memory is fixed and independent of n, the auxiliary space complexity is O(1). This constant-space property makes Selection Sort particularly well suited for memory-constrained environments such as microcontrollers and embedded systems where no dynamic memory allocation is available and every byte of RAM matters.
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Code Explanation & Analysis</div><div class="divider-line"></div></div>

<div class="grid">
  <div class="panel full accent-cyan">
    <h2><span class="icon">▶</span> HOW THE ALGORITHM WORKS — STEP BY STEP</h2>
    Selection Sort works by repeatedly picking the smallest remaining element and placing it at the correct position, one at a time, until the entire array is sorted. The process is simple, predictable, and always takes the same number of steps regardless of how the data is arranged.

    Before sorting begins, the function first checks whether the input is actually sortable. If the array is empty or contains only a single element, there is nothing to do and it stops immediately. This is a defensive measure — it ensures the program behaves correctly even when given unusual or invalid input. An array of zero or one element is already considered sorted by definition, so no work is needed.

    The sorting itself is driven by an outer loop that runs exactly n minus one times, where n is the number of elements. Each pass through this outer loop is responsible for finding the next smallest element and locking it permanently into its final position. After n minus one passes, all but the last element have been placed correctly, and since all smaller elements are already settled, the last element must be the largest, so it automatically sits in the right place without any extra work.

    Inside each pass, a second inner loop scans through every element that has not yet been sorted, from the current position all the way to the end of the array. As it moves forward, it keeps a record of where the smallest value seen so far is located. Each element is compared against this current known minimum, and whenever something smaller is found, the record is updated to point to that new location. This scan is always complete — every remaining element is checked without exception — which is why Selection Sort always makes the same number of comparisons no matter the input.

    Once the inner scan completes, the algorithm knows exactly where the true smallest remaining element is. Before moving it, there is a check: if that smallest element already happens to be sitting exactly where it needs to be, no exchange is performed. This saves three memory write operations in cases where the element is already in place. When a swap is needed, a temporary holding variable preserves one of the values while the two positions exchange their contents — a classic three-step exchange that guarantees no data is ever overwritten and lost.

    The swap itself is handled by a small dedicated helper kept separate from the main logic for clarity. This helper is designed to be merged directly into the surrounding code by the compiler, so there is no extra cost from calling it as a separate function. After each outer pass completes, the sorted region of the array grows by one element, and the process repeats until the entire array is ordered.
  </div>
</div>

<div class="grid grid-2">
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> VARIABLE ROLES EXPLAINED</h2>
    Outer counter — Represents the current boundary between the sorted and unsorted halves. After each outer pass it advances by one, permanently locking one more element in its final sorted position. It never moves backward.

    Inner counter — Scans the unsorted half from just after the outer boundary all the way to the last element. It always moves forward and never revisits any position already scanned in that pass.

    Minimum tracker — Remembers the location of the smallest element encountered during the inner scan. It starts at the outer boundary position and updates whenever a smaller element is found further along. At the end of the inner scan it points to the true minimum.

    Temporary holder — A single extra variable used only during the exchange step. It briefly holds one value so the other can overwrite it without being lost. It is the only additional memory the algorithm needs beyond the input array itself.

    Only four extra integer variables are ever used at any point, and none of them grow in size or count as the input grows larger. This is precisely why the auxiliary space requirement is constant at O(1) regardless of how large the array is.
  </div>
  <div class="panel accent-pink">
    <h2><span class="icon">▶</span> TRACE EXAMPLE: A = [64, 25, 12, 22]</h2>
    Pass 1 — The unsorted region is the entire array. Scanning from position 1 to 3 finds the value 12 at index 2 as the minimum. It is swapped with the value at index 0. The array becomes 12, 25, 64, 22. The sorted region now holds one element.

    Pass 2 — The unsorted region starts at index 1. Scanning from index 2 to 3 finds 22 at index 3 as the minimum. It is swapped with the value at index 1. The array becomes 12, 22, 64, 25. The sorted region now holds two elements.

    Pass 3 — The unsorted region starts at index 2. Scanning finds 25 at index 3 as the minimum. It is swapped with the value at index 2. The array becomes 12, 22, 25, 64. The sorted region now holds three elements.

    End — Only one element remains at index 3. Since all smaller elements are already to its left, the value 64 is automatically in its correct position. The algorithm ends. Total comparisons made: 3 plus 2 plus 1 equals 6, which equals n times n minus 1 divided by 2 for n equal to 4. The formula is confirmed exactly.
  </div>
</div>

<div class="divider"><div class="divider-line"></div><div class="divider-label">// Graph Analysis</div><div class="divider-line"></div></div>

<div class="grid grid-2">
  <div class="panel accent-cyan">
    <h2><span class="icon">▶</span> GRAPH INTERPRETATION</h2>
    The complexity graph plots the number of comparisons against the input size n. Because Selection Sort always makes exactly n(n-1)/2 comparisons regardless of the order of the input, the best case, average case, and worst case all trace the exact same curve. There is no variation between them.

    This single overlapping parabola distinguishes Selection Sort from algorithms like Insertion Sort and Bubble Sort, which can terminate early on already-sorted input and therefore show a lower best-case curve. Selection Sort never benefits from favorable input ordering — it always does the full work.

    The shape of the curve confirms the Theta of n squared growth rate. As n doubles, the number of operations quadruples, which is the defining characteristic of quadratic growth.
  </div>
  <div class="panel accent-purple">
    <h2><span class="icon">▶</span> WHEN TO USE SELECTION SORT</h2>
    Selection Sort is the right choice when memory writes are expensive or limited. Because it makes at most n minus one swaps — one per pass — it minimises the number of write operations to the array. This matters in hardware like Flash memory or EEPROM, where each write wears down the storage medium and has a finite lifetime. Bubble Sort by contrast can make up to n squared swaps, making it far more damaging to write-sensitive storage.

    Selection Sort should be avoided when the input is large or when it is likely to be already partially sorted. For those cases Insertion Sort is a strictly better choice: it adapts to the order of the input and can sort a nearly-sorted array in close to linear time, whereas Selection Sort will always take the same quadratic time regardless.

    The upper bound on swaps — at most n minus one — is actually the theoretical minimum for any comparison-based in-place sorting algorithm, making Selection Sort uniquely efficient in terms of write operations even though it is not efficient in terms of comparisons.
  </div>
</div>

</div>
</div>
