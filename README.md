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

## Where your section is

`index.html` is one file. Every section opens with a banner comment naming the track
that owns it. **Search for the banner text** rather than trusting the line numbers —
they drift as we edit, the banners don't.

| Search for this | Lines | Owner | What's in it |
|---|---|---|---|
| `CONSTANTS - every value sourced` | 138–231 | Krittika | Every number in the model, each with its source in a comment. **Change numbers only here.** |
| `scale calibration (Track F)` | 143–154 | Devavrath | Cell size, tick length, the three diffusivities |
| `pure science functions` | 232–253 | Krittika | `awFromM` (isotherm), `muGrowth` / `daysToMould` (growth model), `respiration` |
| `PHYSICS  (Track B)` | 254–316 | Mohit | `diffuse` (the solver + stability rule), `step` (respiration coupling), `inject` |
| `SENSING + DECISION  (Track C)` | 317–407 | Vishal | `readSensors` (noise + quantization), `anomaly`, `decide` |
| `CONVENTIONAL INSPECTION ARM  (Track E)` | 408–427 | Netra | `inspect` — the comparison baseline and its assumptions |
| `RENDERING  (Track A)` | 483–693 | Chiranjib | Heatmap, hand-rolled charts, timeline |
| `SELF-CHECK` | 738–808 | Chiranjib | Every assertion, including the P3 exit test |

Reading it on GitHub is easiest — the file view lets you jump straight to a line range,
and the banner comments are searchable with the `/` key.

Three ways in, pick whichever suits you:

- **On GitHub** — open `index.html`, press `/`, paste the banner text from the table
- **In an editor** — Ctrl+F the banner text
- **From the terminal** — `grep -n "PHYSICS  (Track B)" index.html`

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
