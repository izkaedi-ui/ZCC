# ZCC Oneirogenesis v2 — Evolution Report

**Generated**: 2026-04-10T21:31:02.738716+00:00

## Summary

| Metric | Value |
|--------|-------|
| Global Generation | 10 |
| Total Survived | 19 |
| Total Rejected | 1 |
| Algorithms Discovered | 10 |
| Blacklisted Patterns | 0 |

## Lineage

| Gen | Island | Hash | Mutations | Δ Score | Timestamp |
|-----|--------|------|-----------|---------|----------|
| G0001 | I0 | `f0b49a19607d` | Swap independent instructions to reduce pipeline stall | -23.7 | 2026-04-10T21:17:59 |
| G0002 | I0 | `8f8d76a99033` | Swap independent instructions to reduce pipeline stall | -9.0 | 2026-04-10T21:18:11 |
| G0003 | I0 | `518b553938a7` | Swap independent instructions to reduce pipeline stall, Replace movq $0 with xorq %rax,%rax (+1) | -8.3 | 2026-04-10T21:18:24 |
| G0004 | I0 | `89c700bb2949` | Swap independent instructions to reduce pipeline stall, Replace movq $0 with xorq %rax,%rax (+1) | -14.1 | 2026-04-10T21:18:36 |
| G0005 | I0 | `710ed6591b08` | Replace movq $0 with xorq %rax,%rax, Swap independent instructions to reduce pipeline stall (+1) | -40.8 | 2026-04-10T21:18:49 |
| G0006 | I0 | `ed8479004b7b` | Replace movq $0 with xorq %rax,%rax, Swap independent instructions to reduce pipeline stall | -10.9 | 2026-04-10T21:19:02 |
| G0007 | I0 | `dad1daadecb9` | Swap independent instructions to reduce pipeline stall | -9.4 | 2026-04-10T21:19:14 |
| G0008 | I0 | `706d316b9266` | Replace movq $0 with xorq %rax,%rax | -4.9 | 2026-04-10T21:19:27 |
| G0009 | I0 | `467d2f9ecc60` | Swap independent instructions to reduce pipeline stall, Replace movq $0 with xorq %rax,%rax | -8.9 | 2026-04-10T21:19:56 |
| G0010 | I0 | `b201697ec5d9` | Sweep: replace ALL 189 imulq $2^n with shlq $n (3→1 cycle), Replace cmpq $0,%rax with testq %rax,%rax | -219.8 | 2026-04-10T21:30:17 |

## Discovered Algorithms

- `QAlgo-Dream-G1` → [`QAlgo-Dream-G1.json`](journal/QAlgo-Dream-G1.json)
- `QAlgo-Dream-G2` → [`QAlgo-Dream-G2.json`](journal/QAlgo-Dream-G2.json)
- `QAlgo-Dream-G3` → [`QAlgo-Dream-G3.json`](journal/QAlgo-Dream-G3.json)
- `QAlgo-Dream-G4` → [`QAlgo-Dream-G4.json`](journal/QAlgo-Dream-G4.json)
- `QAlgo-Dream-G5` → [`QAlgo-Dream-G5.json`](journal/QAlgo-Dream-G5.json)
- `QAlgo-Dream-G6` → [`QAlgo-Dream-G6.json`](journal/QAlgo-Dream-G6.json)
- `QAlgo-Dream-G7` → [`QAlgo-Dream-G7.json`](journal/QAlgo-Dream-G7.json)
- `QAlgo-Dream-G8` → [`QAlgo-Dream-G8.json`](journal/QAlgo-Dream-G8.json)
- `QAlgo-Dream-G9` → [`QAlgo-Dream-G9.json`](journal/QAlgo-Dream-G9.json)
- `QAlgo-Dream-G10` → [`QAlgo-Dream-G10.json`](journal/QAlgo-Dream-G10.json)

## Fitness History

```
G0001 I0: score=2426899
G0002 I0: score=2426900
G0003 I0: score=2426902
G0004 I0: score=2426903
G0005 I0: score=2426909
G0006 I0: score=2426907
G0007 I0: score=2426911
G0008 I0: score=2426914
G0009 I0: score=2426913
G0010 I0: score=2429644
```
