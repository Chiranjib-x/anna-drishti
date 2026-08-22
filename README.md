# Anna-Drishti

Interactive digital twin of a grain stack, for the Smart India Hackathon 2026 internal final at VIT.

A judge injects a spoilage hotspot; simulated physics evolves; virtual sensors read it with
realistic noise; the decision engine detects, predicts and recommends. A comparison mode shows
what conventional peripheral inspection would have caught, and when.

## Running it

Open `index.html` in a browser. That is the whole thing — no build, no server, no install,
no network. It is deliberately one self-contained file with zero external references, because
the venue wifi will fail.

The self-check runs on load. **The pill in the header must be green** before you trust anything
on screen; if it is red it names the assertion that failed.

Then press **Deep interior pocket** and let it run.

## Files

| File | What it is |
|---|---|
| `index.html` | The demo. Physics, sensing, decision engine, rendering, self-check. |
| `build-board.html` | Phase status board — what's done, what's left, who owns what. |

## How it is laid out

`index.html` is one file, sectioned in this order:

- `CONSTANTS` — every value carries its source in a comment. **Change numbers only here.**
- Pure science functions — water activity, mould kinetics, respiration
- Physics — diffusion solver, respiration coupling, CO₂ venting
- Sensing and decision — noisy sampling, anomaly detection, `decide()`
- Conventional inspection arm — the comparison baseline
- Rendering — canvas heatmap, hand-rolled charts, timeline
- `selfCheck()` — the assertions

## Track ownership

| Track | Person | Owns |
|---|---|---|
| A — Integration & interface | Chiranjib | Shell, rendering, controls, merges |
| B — Physics engine | Mohit | Grid, diffusion, respiration |
| C — Sensing & decision | Vishal | Virtual sensors, risk scoring, action cards |
| D — Biology model | Krittika | Thresholds, mould kinetics, realism sign-off |
| E — Baseline & evidence | Netra | Conventional-inspection arm, the comparison number |
| F — Deployment design | Devavrath | Scale calibration, transport constants, probe geometry |

Chiranjib is merge authority. Nobody force-overwrites `main`.

## Before changing a constant

Run the self-check. If it goes red, the change broke something the model depends on —
read the failed assertion before overriding it. Several constants are load-bearing in
non-obvious ways: the system is linear in respiration, so scaling `R0` does not change
the signal-to-background ratio, and the diffusivity *ratios* matter far more than their
absolute values.
