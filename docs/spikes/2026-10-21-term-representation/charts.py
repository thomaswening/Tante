"""Draws the charts of the term-representation spike report from the numbers of CI run
39512843746 (see ../2026-10-21-term-representation.md). Writes a light and a dark SVG
of each chart next to this script.

    python charts.py
"""
from pathlib import Path

import matplotlib

matplotlib.use("svg")
import matplotlib.pyplot as plt  # noqa: E402

HERE = Path(__file__).parent
plt.rcParams["svg.fonttype"] = "none"  # keep text as text
plt.rcParams["svg.hashsalt"] = "tante"  # reproducible ids

THEMES = {
    "light": dict(surface="#fcfcfb", ink="#0b0b0b", ink2="#52514e", muted="#898781",
                  grid="#e1e0d9", axis="#c3c2b7", series=["#2a78d6", "#eb6834", "#1baf7a"]),
    "dark": dict(surface="#1a1a19", ink="#f0efec", ink2="#c3c2b7", muted="#898781",
                 grid="#2c2c2a", axis="#383835", series=["#3987e5", "#d95926", "#199e70"]),
}
FONT = ["system-ui", "Segoe UI", "Helvetica", "Arial", "sans-serif"]
WORKLOADS = ["poly4", "poly20", "bigcoef", "nested"]
SYSTEMS = ["Linux, GCC\n(AMD EPYC 9V74)", "macOS, Clang\n(Apple M2 Pro)", "Windows, MSVC\n(AMD EPYC 7763)"]

# Bytes per term: flat 32-bit, flat 64-bit, tree (Linux, heap plus 16-byte pointer).
BYTES = {"flat, 32-bit words": [63, 192, 99, 160],
         "flat, 64-bit words": [126, 384, 158, 320],
         "tree (Linux)": [222, 352, 238, 705]}

# Sort and merge, ns per input term, 10^6 terms: [system][series][workload].
SORT = [
    {"flat, 32-bit": [519, 796, 615, 960], "flat, 64-bit": [581, 832, 651, 985], "tree": [786, 888, 837, 1567]},
    {"flat, 32-bit": [332, 464, 412, 632], "flat, 64-bit": [365, 560, 493, 686], "tree": [555, 706, 714, 1561]},
    {"flat, 32-bit": [697, 1013, 789, 1165], "flat, 64-bit": [813, 1337, 905, 1529], "tree": [1358, 1601, 1729, 4124]},
]

# Module call, time relative to the direct call: [system][series][case].
CASES = ["read\npoly4", "read\npoly20", "write\npoly4", "write\npoly20"]
CALLS = [
    {"C, one call per symbol": [4.52, 7.81, 1.79, 1.75], "C-bulk, one call per term": [2.53, 2.81, 1.40, 1.21]},
    {"C, one call per symbol": [2.29, 3.97, 1.19, 1.30], "C-bulk, one call per term": [2.77, 1.86, 1.43, 1.72]},
    {"C, one call per symbol": [2.63, 3.19, 1.44, 1.65], "C-bulk, one call per term": [2.61, 2.55, 1.79, 1.88]},
]


def style(ax, t):
    ax.set_facecolor(t["surface"])
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(t["axis"])
    ax.tick_params(colors=t["muted"], labelcolor=t["ink2"], length=0, labelsize=9)
    ax.yaxis.grid(True, color=t["grid"], linewidth=1)
    ax.set_axisbelow(True)


def grouped(ax, t, groups, series, width=0.24, labels=True, fmt="{:.0f}"):
    n = len(series)
    for i, (name, values) in enumerate(series.items()):
        xs = [g + (i - (n - 1) / 2) * width for g in range(len(groups))]
        # A 2px surface-coloured edge gives the gap between touching bars.
        bars = ax.bar(xs, values, width, color=t["series"][i], edgecolor=t["surface"], linewidth=2, label=name)
        if labels:
            for b, v in zip(bars, values):
                ax.annotate(fmt.format(v), (b.get_x() + b.get_width() / 2, b.get_height()),
                            xytext=(0, 3), textcoords="offset points", ha="center", va="bottom",
                            fontsize=8, color=t["ink2"])
    ax.set_xticks(range(len(groups)), groups)


def legend(fig, ax, t, ncol):
    leg = fig.legend(*ax.get_legend_handles_labels(), loc="upper left", bbox_to_anchor=(0.01, 0.9),
                     ncol=ncol, frameon=False, fontsize=9, handlelength=1, handleheight=1)
    for text in leg.get_texts():
        text.set_color(t["ink"])


def title(fig, t, main, sub):
    fig.text(0.01, 0.98, main, fontsize=12, fontweight="bold", color=t["ink"], va="top")
    fig.text(0.01, 0.935, sub, fontsize=9, color=t["ink2"], va="top")


def bytes_chart(mode, t):
    fig, ax = plt.subplots(figsize=(8, 4.2), facecolor=t["surface"])
    style(ax, t)
    grouped(ax, t, WORKLOADS, BYTES)
    ax.set_ylabel("bytes per term", color=t["ink2"], fontsize=9)
    title(fig, t, "Bytes per term", "Lower is better. The tree needs 1.6 to 4.4 times the flat 32-bit encoding.")
    legend(fig, ax, t, 3)
    fig.subplots_adjust(top=0.78, bottom=0.1, left=0.08, right=0.99)
    fig.savefig(HERE / f"bytes-per-term-{mode}.svg", facecolor=t["surface"])
    plt.close(fig)


def sort_chart(mode, t):
    fig, axes = plt.subplots(1, 3, figsize=(10, 4.4), sharey=True, facecolor=t["surface"])
    for ax, system, series in zip(axes, SYSTEMS, SORT):
        style(ax, t)
        grouped(ax, t, WORKLOADS, series, width=0.26, labels=False)
        ax.set_title(system, fontsize=9, color=t["ink"], loc="left")
        ax.tick_params(axis="x", labelsize=8)
    axes[0].set_ylabel("ns per input term", color=t["ink2"], fontsize=9)
    title(fig, t, "Sorting and merging 10⁶ terms",
          "Lower is better. Flat 32-bit is fastest everywhere; the tree is slowest, most of all with nested functions.")
    legend(fig, axes[0], t, 3)
    fig.subplots_adjust(top=0.72, bottom=0.08, left=0.07, right=0.99, wspace=0.08)
    fig.savefig(HERE / f"sort-time-{mode}.svg", facecolor=t["surface"])
    plt.close(fig)


def calls_chart(mode, t):
    fig, axes = plt.subplots(1, 3, figsize=(10, 4.4), sharey=True, facecolor=t["surface"])
    for ax, system, series in zip(axes, SYSTEMS, CALLS):
        style(ax, t)
        grouped(ax, t, CASES, series, width=0.32, fmt="{:.1f}")
        ax.axhline(1.25, color=t["ink2"], linewidth=1, label="pass rule: 1.25×")
        ax.set_title(system, fontsize=9, color=t["ink"], loc="left")
        ax.tick_params(axis="x", labelsize=8)
    axes[0].set_ylabel("time relative to the direct call (×)", color=t["ink2"], fontsize=9)
    title(fig, t, "Cost of a module call through the C interface",
          "1.0 would be as fast as calling the core directly. Almost every bar is above the 1.25 rule.")
    legend(fig, axes[0], t, 3)
    fig.subplots_adjust(top=0.72, bottom=0.12, left=0.07, right=0.99, wspace=0.08)
    fig.savefig(HERE / f"call-cost-{mode}.svg", facecolor=t["surface"])
    plt.close(fig)


plt.rcParams["font.family"] = FONT
for mode, theme in THEMES.items():
    bytes_chart(mode, theme)
    sort_chart(mode, theme)
    calls_chart(mode, theme)
print("written to", HERE)
