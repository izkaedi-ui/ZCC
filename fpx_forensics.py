#!/usr/bin/env python3
"""
fpx_forensics.py — Distinguish real coercion demand from telemetry contamination.

Before committing coercion-cost decisions based on the heatmap, verify that
the ranked edges are firing from diverse source sites (real demand) rather
than a handful of repeated sites (contamination — typically pointer
arithmetic, array subscript lowering, or type-check probes firing the hook
on non-cast IR ops).

Method: per edge, measure context_hash spread.

  spread_ratio = unique_context_hashes / total_events_on_edge

Decision thresholds:
  spread < 0.05    → CONTAMINATION LIKELY — do NOT commit coercion cost.
                     Move the telemetry hook to a narrower IR point
                     (IR_CAST opcode only, excluding IR_ADD/IR_MUL/IR_PTR).
  0.05 ≤ s < 0.15  → NEEDS REVIEW — human-inspect the top context_hashes.
  spread ≥ 0.15    → REAL DEMAND — safe to commit coercion cost/lowering.

Also reports pass_id clustering — if an edge fires from a single IR pass,
that's another contamination signal (typically a single internal lowering
step emitting on every invocation).

Usage:
    python3 fpx_forensics.py fpx_stress.bin
    python3 fpx_forensics.py fpx_stress.bin --edge EMFP16_LIVE,I32
"""
import argparse
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

import pandas as pd

PACKET_FMT = "<HBBBBHII"
PACKET_SIZE = 16

TYPE_NAMES = {
    12: "TOFP32",
    13: "EMFP16_LIVE",
    14: "SPSQ32_DIFF",
    10: "F32",
    15: "F16",
    3:  "I32",
    2:  "I16",
}
EVENT_NAMES = {
    0x5101: "XFMT_FMA", 0x5102: "COERCE_MISSING", 0x5103: "COERCE_OK",
    0x5106: "ATTRACTOR_STEP", 0x5107: "TILE_CROSS_L0",
    0x5108: "TILE_CROSS_L1", 0x5109: "SPSQ_DIFF_ROLL",
}

# Contamination thresholds
SPREAD_CONTAM_MAX    = 0.05
SPREAD_REVIEW_MAX    = 0.15


def read_packets(path):
    data = Path(path).read_bytes()
    rows = []
    for i in range(0, len(data) - PACKET_SIZE + 1, PACKET_SIZE):
        ev, src, dst, op, fl, pid, ctx, ctr = struct.unpack(
            PACKET_FMT, data[i:i+PACKET_SIZE])
        rows.append({"event": ev, "src": src, "dst": dst, "op": op,
                     "flags": fl, "pass_id": pid,
                     "context_hash": ctx, "counter": ctr})
    return pd.DataFrame(rows)


def classify(spread_ratio):
    if spread_ratio < SPREAD_CONTAM_MAX: return "CONTAMINATION_LIKELY"
    if spread_ratio < SPREAD_REVIEW_MAX: return "NEEDS_REVIEW"
    return "REAL_DEMAND"


def verdict_color(label):
    return {
        "CONTAMINATION_LIKELY": "✗",
        "NEEDS_REVIEW":         "?",
        "REAL_DEMAND":          "✓",
    }.get(label, "·")


def analyze_edge(df, src, dst, verbose=False):
    # Only look at cast/coerce events — other events (ATTRACTOR_STEP,
    # TILE_CROSS) are intra-type and irrelevant here.
    cast_evs = {0x5101, 0x5102, 0x5103}
    e = df[(df["src"] == src) & (df["dst"] == dst) &
           (df["event"].isin(cast_evs))]
    if e.empty:
        return None

    total = len(e)
    unique_ctx = e["context_hash"].nunique()
    spread = unique_ctx / total
    label = classify(spread)

    # Pass-id concentration: if one pass fires >90%, that's a red flag
    pass_counts = e["pass_id"].value_counts()
    top_pass_share = pass_counts.iloc[0] / total

    # Top context hashes and their share
    top_ctx = e["context_hash"].value_counts().head(5)

    report = {
        "edge":          f"{TYPE_NAMES.get(src,'?')}→{TYPE_NAMES.get(dst,'?')}",
        "total_events":  total,
        "unique_ctx":    unique_ctx,
        "spread_ratio":  spread,
        "classification": label,
        "top_pass_id":   int(pass_counts.index[0]),
        "top_pass_share": top_pass_share,
        "n_passes_firing": len(pass_counts),
        "top_ctx_hashes": [(hex(c), int(n)) for c, n in top_ctx.items()],
    }

    if verbose:
        print(f"\n{verdict_color(label)} {report['edge']}")
        print(f"   events:           {total}")
        print(f"   unique contexts:  {unique_ctx}")
        print(f"   spread ratio:     {spread:.4f}   [{label}]")
        print(f"   pass_ids firing:  {len(pass_counts)}  "
              f"(top pass #{report['top_pass_id']} "
              f"carries {top_pass_share:.0%})")
        print(f"   top context hashes:")
        for c, n in top_ctx.items():
            pct = 100 * n / total
            print(f"     {hex(c):>12}  ×{n:<6}  ({pct:.1f}%)")

    return report


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input")
    ap.add_argument("--edge", help="analyze a single edge: SRC,DST names")
    ap.add_argument("--csv", default="fpx_forensics.csv")
    args = ap.parse_args()

    df = read_packets(args.input)
    if df.empty:
        print("no packets", file=sys.stderr); sys.exit(1)

    # Reverse lookup for --edge names
    name_to_id = {v: k for k, v in TYPE_NAMES.items()}

    if args.edge:
        src_name, dst_name = args.edge.split(",")
        src = name_to_id[src_name]; dst = name_to_id[dst_name]
        r = analyze_edge(df, src, dst, verbose=True)
        if r is None:
            print(f"no events for {args.edge}", file=sys.stderr); sys.exit(1)
        return

    # Full sweep — analyze every edge that has cast events
    cast_evs = {0x5101, 0x5102, 0x5103}
    edges = (df[df["event"].isin(cast_evs)]
             .groupby(["src", "dst"]).size()
             .sort_values(ascending=False))

    reports = []
    print(f"\n=== FPX CONTAMINATION FORENSICS ===\n")
    print(f"{'edge':<28} {'events':>8} {'uniq_ctx':>10} "
          f"{'spread':>8}  {'verdict':<22} {'passes':>7}")
    print("-" * 90)
    for (src, dst), _ in edges.items():
        r = analyze_edge(df, src, dst)
        if r is None: continue
        reports.append(r)
        mark = verdict_color(r["classification"])
        print(f"{mark} {r['edge']:<26} {r['total_events']:>8} "
              f"{r['unique_ctx']:>10} {r['spread_ratio']:>8.4f}  "
              f"{r['classification']:<22} {r['n_passes_firing']:>7}")

    rpt_df = pd.DataFrame(reports)
    rpt_df.to_csv(args.csv, index=False)
    print(f"\nfull report → {args.csv}")

    # Summary verdict
    contam = rpt_df[rpt_df["classification"] == "CONTAMINATION_LIKELY"]
    review = rpt_df[rpt_df["classification"] == "NEEDS_REVIEW"]
    real   = rpt_df[rpt_df["classification"] == "REAL_DEMAND"]
    print(f"\nSummary:  {len(real)} real,  {len(review)} review,  "
          f"{len(contam)} contamination-likely")

    if not contam.empty:
        print(f"\n✗  DO NOT commit coercion costs for these edges "
              f"until the hook is narrowed:")
        for _, r in contam.iterrows():
            print(f"     {r['edge']:<28} "
                  f"spread={r['spread_ratio']:.4f}  "
                  f"({r['total_events']} events from {r['unique_ctx']} sites)")
        print(f"\n   Likely fix: narrow ir_tel_fpx_coerce_cost() call sites")
        print(f"   to IR_CAST opcodes only; exclude IR_ADD / IR_MUL /")
        print(f"   pointer-arithmetic lowering paths.")


if __name__ == "__main__":
    main()
