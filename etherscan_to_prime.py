#!/usr/bin/env python3
"""
🔱 Etherscan → PRIME Energy Bridge v1.0.0
═════════════════════════════════════════

Reads the Etherscan batch fetch corpus (training_corpus.json),
extracts structural features from Solidity source code,
and maps them to PRIME energy vectors for EVM training.

Input:  training_corpus.json  (from etherscan_batch_fetch.ts)
Output: prime_training_data.json  (ready for train_evm.py)

Feature Vector (per contract, 19-D matching vuln classes):
  [0]  reentrancy_energy      — external calls × state writes
  [1]  overflow_energy         — unchecked arithmetic density
  [2]  access_control_energy   — missing modifier patterns
  [3]  delegatecall_energy     — delegatecall + storage collision risk
  [4]  selfdestruct_energy     — selfdestruct reachability
  [5]  tx_origin_energy        — tx.origin auth patterns
  [6]  timestamp_energy        — block.timestamp dependencies
  [7]  front_running_energy    — price-sensitive ops without slippage
  [8]  dos_energy              — unbounded loops + external calls
  [9]  flash_loan_energy       — balance checks after external calls
  [10] oracle_energy           — single oracle dependency patterns
  [11] storage_collision       — unstructured proxy storage
  [12] uninitialized_energy    — state vars without initialization
  [13] gas_griefing            — returndata copy + low-level calls
  [14] signature_replay        — ecrecover without nonce
  [15] precision_loss          — division before multiplication
  [16] abi_encoding_energy     — abi.encodePacked collision risk
  [17] unchecked_return        — low-level call return unchecked
  [18] event_emission          — state changes without events

PRIME Hamiltonian Dimensions (per contract, 4-D):
  base_cost        — weighted instruction complexity
  branch_density   — control flow complexity
  call_density     — external interaction surface
  prime_energy     — composite H₀ × (1 + 0.3·branch + 0.5·call)

Usage:
    python3 etherscan_to_prime.py [corpus.json] [output.json]
"""

import json
import re
import sys
import math
from pathlib import Path
from collections import defaultdict


# ═══════════════════════════════════════════════════════════════════
# Solidity Pattern Detectors
# ═══════════════════════════════════════════════════════════════════

class SolidityFeatureExtractor:
    """Extract vulnerability-relevant structural features from Solidity source."""

    # Precompiled regex patterns for performance
    PATTERNS = {
        # Reentrancy indicators
        'external_call': re.compile(
            r'\.(call|send|transfer)\s*[\({]'
            r'|\.call\{value:'
            r'|address\(.*\)\.(call|send|transfer)',
            re.IGNORECASE
        ),
        'state_write_after_call': re.compile(
            r'(\.call|\.send|\.transfer).*?;\s*\n\s*\w+\s*[=\[]',
            re.DOTALL
        ),

        # Overflow / unchecked
        'unchecked_block': re.compile(r'unchecked\s*\{'),
        'arithmetic_op': re.compile(r'[\+\-\*\/]\s*=|[\+\-\*\/]{1,2}(?!=)'),
        'safe_math': re.compile(r'using\s+SafeMath', re.IGNORECASE),

        # Access control
        'onlyowner': re.compile(r'onlyOwner|onlyAdmin|onlyRole|requireOwner', re.IGNORECASE),
        'modifier_def': re.compile(r'modifier\s+\w+'),
        'public_func': re.compile(r'function\s+\w+\s*\([^)]*\)\s*(public|external)'),
        'require_msg_sender': re.compile(r'require\s*\(\s*msg\.sender\s*=='),

        # Delegatecall
        'delegatecall': re.compile(r'\.delegatecall\s*\('),
        'proxy_pattern': re.compile(r'(Proxy|Upgradeable|Implementation)', re.IGNORECASE),

        # Selfdestruct
        'selfdestruct': re.compile(r'selfdestruct\s*\(|suicide\s*\('),

        # tx.origin
        'tx_origin': re.compile(r'tx\.origin'),

        # Timestamp
        'block_timestamp': re.compile(r'block\.timestamp|now\b'),

        # Front-running / MEV
        'price_sensitive': re.compile(
            r'getReserves|getAmountOut|getAmountsOut|swap\s*\('
            r'|price|oracle|feed',
            re.IGNORECASE
        ),
        'slippage_check': re.compile(r'minAmount|amountOutMin|deadline|slippage', re.IGNORECASE),

        # DoS
        'unbounded_loop': re.compile(r'for\s*\(\s*uint\s+\w+\s*=\s*0\s*;\s*\w+\s*<\s*\w+\.length'),
        'loop_external': re.compile(r'for\s*\(.*?\{[^}]*\.(call|send|transfer)', re.DOTALL),

        # Flash loan
        'flash_pattern': re.compile(r'flashLoan|flash|balanceOf.*after', re.IGNORECASE),

        # Oracle
        'single_oracle': re.compile(r'latestRoundData|latestAnswer|getPrice', re.IGNORECASE),
        'chainlink': re.compile(r'AggregatorV3|priceFeed|chainlink', re.IGNORECASE),

        # Storage collision (proxy)
        'storage_slot': re.compile(r'sload|sstore|assembly\s*\{[^}]*slot'),

        # Uninitialized
        'state_var': re.compile(r'(uint|int|address|bool|bytes|string|mapping)\s+\w+\s*;'),
        'constructor_init': re.compile(r'constructor\s*\([^)]*\)\s*\{'),

        # Gas griefing
        'returndata_copy': re.compile(r'returndatacopy|returndatasize'),
        'low_level_call': re.compile(r'\.call\s*\(|\.staticcall\s*\(|\.delegatecall\s*\('),

        # Signature replay
        'ecrecover': re.compile(r'ecrecover\s*\('),
        'nonce_check': re.compile(r'nonces?\[|_nonce|replayGuard', re.IGNORECASE),

        # Precision loss
        'div_before_mul': re.compile(r'\/\s*\w+\s*\*'),
        'division': re.compile(r'\/(?!=)'),

        # ABI encoding
        'encode_packed': re.compile(r'abi\.encodePacked\s*\('),

        # Unchecked return
        'low_level_unchecked': re.compile(
            r'(\.call|\.send|\.delegatecall)\s*\([^;]*\)\s*;'
        ),
        'success_check': re.compile(r'\(bool\s+success'),

        # Events
        'emit_event': re.compile(r'emit\s+\w+\s*\('),
        'event_def': re.compile(r'event\s+\w+\s*\('),

        # General complexity
        'function_def': re.compile(r'function\s+\w+'),
        'if_else': re.compile(r'\bif\s*\(|\belse\b'),
        'require_assert': re.compile(r'require\s*\(|assert\s*\(|revert\s'),
        'mapping_def': re.compile(r'mapping\s*\('),
        'interface_def': re.compile(r'interface\s+\w+'),
        'library_use': re.compile(r'using\s+\w+\s+for'),
        'inheritance': re.compile(r'contract\s+\w+\s+is\s+'),
    }

    @classmethod
    def extract(cls, source_code: str) -> dict:
        """Extract all feature counts from Solidity source code."""
        if not source_code or not isinstance(source_code, str):
            return cls._empty_features()

        features = {}
        for name, pattern in cls.PATTERNS.items():
            features[name] = len(pattern.findall(source_code))

        # Derived metrics
        total_lines = max(source_code.count('\n'), 1)
        features['total_lines'] = total_lines
        features['func_count'] = features['function_def']
        features['complexity'] = features['if_else'] + features['require_assert']

        return features

    @classmethod
    def _empty_features(cls):
        return {name: 0 for name in cls.PATTERNS}


# ═══════════════════════════════════════════════════════════════════
# PRIME Energy Vector Builder
# ═══════════════════════════════════════════════════════════════════

class PRIMEEnergyMapper:
    """Map Solidity features to 19-D vulnerability energy + 4-D PRIME Hamiltonian."""

    # Vulnerability class names (matching solidity-vulnerability-energy-signatures)
    VULN_CLASSES = [
        'reentrancy',           # 0
        'integer_overflow',     # 1
        'access_control',       # 2
        'delegatecall_inject',  # 3
        'selfdestruct',         # 4
        'tx_origin_auth',       # 5
        'timestamp_depend',     # 6
        'front_running',        # 7
        'dos_unbounded',        # 8
        'flash_loan_attack',    # 9
        'oracle_manipulation',  # 10
        'storage_collision',    # 11
        'uninitialized_var',    # 12
        'gas_griefing',         # 13
        'signature_replay',     # 14
        'precision_loss',       # 15
        'abi_collision',        # 16
        'unchecked_return',     # 17
        'missing_events',       # 18
    ]

    @classmethod
    def compute_energy_vector(cls, features: dict) -> list:
        """
        Map raw feature counts to 19-D normalized energy vector.
        Each dimension ∈ [0, 1] representing vulnerability exposure.
        """
        total_lines = max(features.get('total_lines', 1), 1)
        func_count = max(features.get('func_count', 1), 1)

        def norm(val, scale=1.0):
            """Sigmoid normalization to [0, 1]."""
            return 1.0 / (1.0 + math.exp(-scale * val))

        def density(count, base=None):
            """Feature density normalized by code size."""
            b = base or total_lines
            return count / max(b, 1)

        vec = [0.0] * 19

        # [0] Reentrancy: external calls × state writes, inversely weighted by checks
        ext_calls = features.get('external_call', 0)
        state_after = features.get('state_write_after_call', 0)
        checks = features.get('require_assert', 0)
        vec[0] = norm(ext_calls * 2 + state_after * 4 - checks * 0.5, 0.3)

        # [1] Integer overflow: arithmetic ops outside unchecked blocks / SafeMath
        arith = features.get('arithmetic_op', 0)
        unchecked = features.get('unchecked_block', 0)
        safe_math = features.get('safe_math', 0)
        vec[1] = norm(unchecked * 3 + arith * 0.1 - safe_math * 5, 0.2)

        # [2] Access control: public funcs without modifiers
        public = features.get('public_func', 0)
        modifiers = features.get('onlyowner', 0) + features.get('modifier_def', 0)
        msg_sender = features.get('require_msg_sender', 0)
        vec[2] = norm(public * 1.5 - modifiers * 2 - msg_sender * 1.5, 0.15)

        # [3] Delegatecall injection
        deleg = features.get('delegatecall', 0)
        proxy = features.get('proxy_pattern', 0)
        vec[3] = norm(deleg * 5 + proxy * 0.5, 0.4)

        # [4] Selfdestruct
        vec[4] = norm(features.get('selfdestruct', 0) * 8, 0.5)

        # [5] tx.origin auth
        vec[5] = norm(features.get('tx_origin', 0) * 6, 0.5)

        # [6] Timestamp dependency
        vec[6] = norm(features.get('block_timestamp', 0) * 2, 0.3)

        # [7] Front-running / MEV exposure
        price = features.get('price_sensitive', 0)
        slippage = features.get('slippage_check', 0)
        vec[7] = norm(price * 2 - slippage * 3, 0.25)

        # [8] DoS: unbounded loops
        loops = features.get('unbounded_loop', 0)
        loop_ext = features.get('loop_external', 0)
        vec[8] = norm(loops * 3 + loop_ext * 6, 0.3)

        # [9] Flash loan
        vec[9] = norm(features.get('flash_pattern', 0) * 4, 0.4)

        # [10] Oracle manipulation
        single = features.get('single_oracle', 0)
        chainlink = features.get('chainlink', 0)
        vec[10] = norm(single * 2 - chainlink * 0.5, 0.3)

        # [11] Storage collision (proxy patterns)
        slots = features.get('storage_slot', 0)
        vec[11] = norm(slots * 3 + features.get('proxy_pattern', 0) * 1, 0.3)

        # [12] Uninitialized variables
        state_vars = features.get('state_var', 0)
        constructors = features.get('constructor_init', 0)
        vec[12] = norm(state_vars * 0.5 - constructors * 2, 0.2)

        # [13] Gas griefing
        returndata = features.get('returndata_copy', 0)
        low_level = features.get('low_level_call', 0)
        vec[13] = norm(returndata * 4 + low_level * 1, 0.3)

        # [14] Signature replay
        ecr = features.get('ecrecover', 0)
        nonce = features.get('nonce_check', 0)
        vec[14] = norm(ecr * 5 - nonce * 4, 0.4)

        # [15] Precision loss
        div_mul = features.get('div_before_mul', 0)
        divs = features.get('division', 0)
        vec[15] = norm(div_mul * 4 + divs * 0.3, 0.3)

        # [16] ABI encoding collision
        vec[16] = norm(features.get('encode_packed', 0) * 5, 0.5)

        # [17] Unchecked return
        unchecked_ret = features.get('low_level_unchecked', 0)
        success = features.get('success_check', 0)
        vec[17] = norm(unchecked_ret * 3 - success * 4, 0.3)

        # [18] Missing events
        events_emitted = features.get('emit_event', 0)
        events_defined = features.get('event_def', 0)
        state_writes = features.get('state_write_after_call', 0) + features.get('state_var', 0)
        vec[18] = norm(state_writes * 0.5 - events_emitted * 1 - events_defined * 0.5, 0.15)

        return [round(v, 6) for v in vec]

    @classmethod
    def compute_prime_hamiltonian(cls, features: dict) -> dict:
        """
        Compute 4-D PRIME Hamiltonian dimensions from structural features.
        Mirrors zcc_ir_opts.py prime_energy_score() dimensionality.
        """
        total_lines = max(features.get('total_lines', 1), 1)
        func_count = max(features.get('func_count', 1), 1)

        # Base cost: weighted complexity
        arith = features.get('arithmetic_op', 0)
        ext_calls = features.get('external_call', 0) + features.get('low_level_call', 0)
        checks = features.get('require_assert', 0)
        mappings = features.get('mapping_def', 0)

        base_cost = (
            arith * 1.0 +
            ext_calls * 8.0 +
            checks * 2.0 +
            mappings * 3.0 +
            features.get('state_var', 0) * 1.5 +
            features.get('modifier_def', 0) * 2.0 +
            func_count * 1.0
        )

        # Branch density
        branches = features.get('if_else', 0)
        branch_density = branches / max(total_lines, 1)

        # Call density (external interaction surface)
        calls = ext_calls + features.get('delegatecall', 0)
        call_density = calls / max(func_count, 1)

        # PRIME composite energy
        prime_energy = base_cost * (1.0 + 0.3 * branch_density + 0.5 * call_density)

        return {
            'base_cost': round(base_cost, 2),
            'branch_density': round(branch_density, 4),
            'call_density': round(call_density, 4),
            'prime_energy': round(prime_energy, 2),
        }


# ═══════════════════════════════════════════════════════════════════
# Corpus Processor
# ═══════════════════════════════════════════════════════════════════

def process_corpus(corpus_path: str) -> list:
    """
    Read Etherscan batch fetch output and convert to PRIME training data.

    Expected corpus format (from etherscan_batch_fetch.ts):
    [
      {
        "address": "0x...",
        "name": "Uniswap V2 Router",
        "category": "DEX",
        "source": "pragma solidity ^0.8.0; ...",
        "abi": [...],
        "bytecode": "0x..."
      },
      ...
    ]
    """
    with open(corpus_path) as f:
        corpus = json.load(f)

    if isinstance(corpus, dict):
        # Handle wrapped format: { "contracts": [...] }
        corpus = corpus.get('contracts', corpus.get('data', [corpus]))

    if not isinstance(corpus, list):
        corpus = [corpus]

    results = []
    extractor = SolidityFeatureExtractor()

    for entry in corpus:
        if not isinstance(entry, dict):
            continue

        address = entry.get('address', 'unknown')
        name = entry.get('name', entry.get('ContractName', entry.get('contract_name', 'Unknown')))
        category = entry.get('category', entry.get('vulnerability_type', 'unknown'))
        source = entry.get('source', entry.get('SourceCode', entry.get('vulnerable_code', '')))

        # Skip entries without source
        if not source or len(source) < 50:
            continue

        # Clean source (handle Etherscan double-encoding)
        if source.startswith('{{') or source.startswith('{'):
            try:
                parsed = json.loads(source.strip('{}'))
                if isinstance(parsed, dict) and 'sources' in parsed:
                    # Multi-file format — concatenate all sources
                    parts = []
                    for fname, fdata in parsed['sources'].items():
                        if isinstance(fdata, dict):
                            parts.append(fdata.get('content', ''))
                        else:
                            parts.append(str(fdata))
                    source = '\n'.join(parts)
            except (json.JSONDecodeError, AttributeError):
                pass

        # Extract features
        features = extractor.extract(source)

        # Compute energy vectors
        energy_19d = PRIMEEnergyMapper.compute_energy_vector(features)
        prime_4d = PRIMEEnergyMapper.compute_prime_hamiltonian(features)

        # Determine dominant vulnerability class
        max_idx = energy_19d.index(max(energy_19d))
        dominant_vuln = PRIMEEnergyMapper.VULN_CLASSES[max_idx]
        dominant_energy = energy_19d[max_idx]

        results.append({
            'address': address,
            'name': name,
            'category': category,
            'source_lines': features.get('total_lines', 0),
            'func_count': features.get('func_count', 0),
            'energy_vector_19d': energy_19d,
            'prime_hamiltonian': prime_4d,
            'dominant_vuln': dominant_vuln,
            'dominant_energy': round(dominant_energy, 4),
            'label': max_idx,
            # Raw features for debugging
            'raw_features': {
                k: v for k, v in features.items()
                if isinstance(v, (int, float)) and v > 0
            },
        })

    return results


# ═══════════════════════════════════════════════════════════════════
# Training Data Formatter
# ═══════════════════════════════════════════════════════════════════

def format_for_training(processed: list) -> dict:
    """
    Format processed corpus into train_evm.py compatible structure.

    Output format:
    {
      "metadata": { ... },
      "samples": [
        {
          "features": [19-D energy + 4-D prime = 23-D vector],
          "label": int (0-18),
          "label_name": str,
          "address": str,
          "name": str
        }
      ],
      "class_names": [19 vulnerability class names],
      "feature_names": [23 feature dimension names]
    }
    """
    feature_names = [
        # 19-D vulnerability energy
        'reentrancy_energy', 'overflow_energy', 'access_control_energy',
        'delegatecall_energy', 'selfdestruct_energy', 'tx_origin_energy',
        'timestamp_energy', 'front_running_energy', 'dos_energy',
        'flash_loan_energy', 'oracle_energy', 'storage_collision',
        'uninitialized_energy', 'gas_griefing', 'signature_replay',
        'precision_loss', 'abi_collision', 'unchecked_return', 'missing_events',
        # 4-D PRIME Hamiltonian
        'prime_base_cost', 'prime_branch_density',
        'prime_call_density', 'prime_energy',
    ]

    samples = []
    class_dist = defaultdict(int)

    for entry in processed:
        # Concatenate 19-D + 4-D = 23-D feature vector
        prime = entry['prime_hamiltonian']
        features = entry['energy_vector_19d'] + [
            prime['base_cost'] / 1000.0,     # normalize to ~[0,1]
            prime['branch_density'],
            prime['call_density'],
            prime['prime_energy'] / 1000.0,   # normalize to ~[0,1]
        ]

        label = entry['label']
        class_dist[entry['dominant_vuln']] += 1

        samples.append({
            'features': features,
            'label': label,
            'label_name': entry['dominant_vuln'],
            'address': entry['address'],
            'name': entry['name'],
        })

    return {
        'metadata': {
            'total_samples': len(samples),
            'feature_dim': 23,
            'num_classes': 19,
            'class_distribution': dict(class_dist),
            'source': 'etherscan_batch_fetch + PRIME energy mapping',
        },
        'samples': samples,
        'class_names': PRIMEEnergyMapper.VULN_CLASSES,
        'feature_names': feature_names,
    }


# ═══════════════════════════════════════════════════════════════════
# Main
# ═══════════════════════════════════════════════════════════════════

def main():
    corpus_path = sys.argv[1] if len(sys.argv) > 1 else 'training_corpus.json'
    output_path = sys.argv[2] if len(sys.argv) > 2 else 'prime_training_data.json'

    print("🔱 Etherscan → PRIME Energy Bridge v1.0.0")
    print("=" * 60)

    if not Path(corpus_path).exists():
        print(f"[ERROR] Corpus not found: {corpus_path}")
        print("  Run etherscan_batch_fetch.ts first to generate the corpus.")
        print("  Or provide a path: python3 etherscan_to_prime.py <corpus.json>")
        sys.exit(1)

    print(f"  Corpus: {corpus_path}")

    # Process
    processed = process_corpus(corpus_path)
    print(f"  Contracts extracted: {len(processed)}")

    if not processed:
        print("[ERROR] No valid contracts found in corpus.")
        sys.exit(1)

    # Format for training
    training_data = format_for_training(processed)

    # Stats
    meta = training_data['metadata']
    print(f"\n  Training Data Summary:")
    print(f"    Samples:     {meta['total_samples']}")
    print(f"    Feature dim: {meta['feature_dim']}")
    print(f"    Classes:     {meta['num_classes']}")
    print(f"\n  Class Distribution:")
    for cls, count in sorted(meta['class_distribution'].items(), key=lambda x: -x[1]):
        bar = '█' * min(count, 30)
        print(f"    {cls:<25} {count:>4}  {bar}")

    # PRIME energy stats
    energies = [p['prime_hamiltonian']['prime_energy'] for p in processed]
    print(f"\n  🔱 PRIME Energy Stats:")
    print(f"    Min:  {min(energies):.1f}")
    print(f"    Max:  {max(energies):.1f}")
    print(f"    Mean: {sum(energies)/len(energies):.1f}")

    # Top 5 highest energy contracts
    by_energy = sorted(processed, key=lambda x: -x['prime_hamiltonian']['prime_energy'])
    print(f"\n  🔱 Highest Energy Contracts:")
    for c in by_energy[:5]:
        p = c['prime_hamiltonian']
        print(f"    {c['name']:<35} E={p['prime_energy']:>8.1f}  "
              f"base={p['base_cost']:>6.1f}  br={p['branch_density']:.3f}  "
              f"call={p['call_density']:.3f}")

    # Write
    with open(output_path, 'w') as f:
        json.dump(training_data, f, indent=2)

    print(f"\n  ✅ Written: {output_path}")
    print(f"     {meta['total_samples']} samples × {meta['feature_dim']}-D features")

    # Ouroboros-scrapable summary
    print(f"\n🔱 ETHERSCAN-PRIME: {meta['total_samples']} contracts → "
          f"{meta['feature_dim']}-D vectors, "
          f"{meta['num_classes']} classes, "
          f"E_max={max(energies):.1f}")


if __name__ == '__main__':
    main()
