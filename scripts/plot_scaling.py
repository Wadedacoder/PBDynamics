"""Plot the results of `PBDynamics --benchmark` (bench/scaling.csv).

Writes:
  docs/scaling.png   - median frame time vs particle count, one line per substep count
  docs/breakdown.png - where the frame time goes at 5 substeps (the interactive default)

Usage: python scripts/plot_scaling.py [bench/scaling.csv]
"""
import csv
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

SURFACE = "#fcfcfb"
INK = "#0b0b0b"
INK_2 = "#52514e"
MUTED = "#898781"
GRID = "#e1e0d9"
AXIS = "#c3c2b7"
SUBSTEP_COLORS = {1: "#6da7ec", 5: "#2a78d6", 10: "#104281"}  # ordinal blue ramp
STAGES = [  # (csv column, label, color) - categorical slots in fixed order
    ("constraints_mean_ms", "Constraint solve (CPU)", "#2a78d6"),
    ("integrate_mean_ms", "Integrate (CPU)", "#eb6834"),
    ("normals_mean_ms", "Normals (CPU)", "#1baf7a"),
    ("upload_mean_ms", "VBO rebuild + upload (CPU)", "#eda100"),
    ("gpu_draw_mean_ms", "Draw (GPU)", "#e87ba4"),
]
REMAINDER_COLOR = "#d6d4cc"

plt.rcParams.update({
    "figure.facecolor": SURFACE, "axes.facecolor": SURFACE, "savefig.facecolor": SURFACE,
    "axes.edgecolor": AXIS, "axes.labelcolor": INK_2, "xtick.color": MUTED, "ytick.color": MUTED,
    "text.color": INK, "font.size": 10, "axes.spines.top": False, "axes.spines.right": False,
})

path = sys.argv[1] if len(sys.argv) > 1 else "bench/scaling.csv"
rows = [{k: float(v) for k, v in r.items()} for r in csv.DictReader(open(path))]
fmt_int = FuncFormatter(lambda x, _: f"{int(x):,}")

# --- Frame time vs particle count -----------------------------------------------------------
fig, ax = plt.subplots(figsize=(7.5, 4.6), dpi=150)
ax.grid(True, which="major", color=GRID, linewidth=0.8)
ax.set_axisbelow(True)
for s, color in SUBSTEP_COLORS.items():
    pts = sorted((r["particles"], r["frame_p50_ms"]) for r in rows if r["substeps"] == s)
    xs, ys = zip(*pts)
    ax.plot(xs, ys, color=color, linewidth=2, marker="o", markersize=6,
            markeredgecolor=SURFACE, markeredgewidth=1.5, label=f"{s} substep{'s' if s > 1 else ''}")
    ax.annotate(f"{s} substep{'s' if s > 1 else ''}", (xs[-1], ys[-1]), xytext=(8, 0),
                textcoords="offset points", va="center", color=INK_2, fontsize=9)
ax.axhline(1000 / 60, color=MUTED, linewidth=1, linestyle=(0, (4, 3)))
ax.annotate("60 fps budget (16.7 ms)", (256, 1000 / 60), xytext=(0, 4), textcoords="offset points",
            color=MUTED, fontsize=9)
ax.set_xscale("log")
ax.set_yscale("log")
ax.xaxis.set_major_formatter(fmt_int)
ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f"{y:g}"))
ax.set_xlim(180, 200000)
ax.set_xlabel("Particles (N x N cloth, 16 to 256 per side)")
ax.set_ylabel("Frame time, median (ms)")
ax.set_title("PBD cloth: CPU frame time vs particle count",
             loc="left", fontsize=11, color=INK, pad=12)
ax.legend(frameon=False, loc="upper left", labelcolor=INK_2)
fig.tight_layout()
fig.savefig("docs/scaling.png")
plt.close(fig)

# --- Where the time goes at 5 substeps ------------------------------------------------------
sub = sorted((r for r in rows if r["substeps"] == 5), key=lambda r: r["particles"])
fig, ax = plt.subplots(figsize=(7.5, 3.8), dpi=150)
labels = [f"{int(r['particles']):,}" for r in sub]
left = [0.0] * len(sub)
gap = dict(edgecolor=SURFACE, linewidth=2)  # 2px surface gap between segments
for col, label, color in STAGES:
    widths = [100 * r[col] / r["frame_mean_ms"] for r in sub]
    ax.barh(labels, widths, left=left, color=color, label=label, height=0.62, **gap)
    left = [l + w for l, w in zip(left, widths)]
remainder = [max(0.0, 100 - l) for l in left]
ax.barh(labels, remainder, left=left, color=REMAINDER_COLOR, label="Other (submission, sync)",
        height=0.62, **gap)
for i, r in enumerate(sub):
    share = 100 * r["constraints_mean_ms"] / r["frame_mean_ms"]
    ax.text(2, i, f"{share:.0f}%", va="center", color=SURFACE, fontsize=9, fontweight="bold")
    ax.text(101.5, i, f"{r['frame_mean_ms']:.1f} ms", va="center", color=INK_2, fontsize=9)
ax.set_xlim(0, 112)
ax.set_xticks([0, 25, 50, 75, 100])
ax.xaxis.set_major_formatter(FuncFormatter(lambda x, _: f"{x:.0f}%"))
ax.set_xlabel("Share of mean frame time (5 substeps)")
ax.set_ylabel("Particles")
ax.set_title("Where the frame time goes", loc="left", fontsize=11, color=INK, pad=12)
ax.legend(frameon=False, loc="upper center", bbox_to_anchor=(0.5, -0.22), ncol=3, fontsize=8.5,
          labelcolor=INK_2)
ax.tick_params(axis="y", length=0)
fig.tight_layout()
fig.savefig("docs/breakdown.png")
plt.close(fig)
print("wrote docs/scaling.png and docs/breakdown.png")
