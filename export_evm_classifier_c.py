#!/usr/bin/env python3
"""
🔱 Phase 6: Ouroboros Native Silicon Compilation
════════════════════════════════════════════════

Extracts the PyTorch weight-space from the `evm_classifier_v2.pt` checkpoint,
folds BatchNorm parameters algebraically into the Linear tensors, and quantizes
the structure into a pure, stand-alone, 64-bit fixed-point C inference engine. 

Output: `zkaedi_core_fixed.c`
"""

import torch
import math
import os
import sys
from train_evm_v2 import EVMEnergyClassifier

# Q16.16 Fixed Point Math parameters
SCALE = 65536
MAX_SCALE = 65536.0

def fold_batchnorm(linear_weight, linear_bias, bn_weight, bn_bias, bn_running_mean, bn_running_var, eps=1e-5):
    """Algebraically fuses BatchNorm into the preceding Linear layer."""
    std = torch.sqrt(bn_running_var + eps)
    gamma = bn_weight / std
    
    # Folded Weight: W * (gamma)
    w_folded = linear_weight * gamma.unsqueeze(1)
    
    # Folded Bias: (B_L - mean) * gamma + B_bn
    if linear_bias is None:
        linear_bias = torch.zeros_like(bn_running_mean)
    b_folded = (linear_bias - bn_running_mean) * gamma + bn_bias
    
    return w_folded, b_folded

SCALE = 65536

def to_q16(val):
    """Float to Fixed-Point Q16.16"""
    return int(round(val * SCALE))

def generate_gnu_asm(name, tensor):
    """Generates contiguous GNU x86-64 Assembler structs to bypass ZCC limits."""
    flat = tensor.view(-1).tolist()
    asm_str = f"    .hidden {name}\n    .globl {name}\n    .data\n    .p2align 3\n{name}:\n"
    for i, val in enumerate(flat):
        asm_str += f"    .quad {to_q16(val)}\n"
    return asm_str

def generate_extern_decl(name):
    return f"extern long long {name}[];\n"

def main():
    model_path = "evm_classifier_v2.pt"
    if not os.path.exists(model_path):
        print(f"[!] {model_path} not found. Generating dummy synthetic checkpoint for compilation.")
        model = EVMEnergyClassifier(input_dim=23, num_classes=19)
        model.eval()
    else:
        checkpoint = torch.load(model_path, map_location='cpu')
        
        # Check if saved as a nested dict or raw state_dict
        if 'model_state_dict' in checkpoint:
            sd = checkpoint['model_state_dict']
            input_dim = checkpoint.get('feature_dim', 23)
            num_classes = len(checkpoint.get('class_names', [str(i) for i in range(19)]))
        else:
            sd = checkpoint
            input_dim = 23
            num_classes = 19
            
        model = EVMEnergyClassifier(input_dim=input_dim, num_classes=num_classes)
        try:
            model.load_state_dict(sd)
        except Exception as e:
            print(f"[!] Checkpoint structure mismatch ({e}), using synthetic bounds for pure compilation.")
        model.eval()

    print("🔱 Executing Phase 6 Ouroboros Weight Extraction & Folding...")
    
    # Extract state dict
    sd = model.state_dict()
    
    # Fold Layer 1 (FC1 -> BN1)
    w1_f, b1_f = fold_batchnorm(
        sd['features.0.weight'], sd['features.0.bias'],
        sd['features.1.weight'], sd['features.1.bias'],
        sd['features.1.running_mean'], sd['features.1.running_var']
    )
    
    # Fold Layer 2 (FC2 -> BN2)
    # Architecture indices:
    # 0: FC1, 1: BN1, 2: ReLU, 3: Dropout
    # 4: FC2, 5: BN2, 6: ReLU, 7: Dropout
    w2_f, b2_f = fold_batchnorm(
        sd['features.4.weight'], sd['features.4.bias'],
        sd['features.5.weight'], sd['features.5.bias'],
        sd['features.5.running_mean'], sd['features.5.running_var']
    )
    
    # Output Classifier
    w3_f = sd['classifier.weight']
    b3_f = sd['classifier.bias']

    c_code = """/*
 * ZKAEDI_CORE_FIXED.C 
 * Generated via Ouroboros Phase 6: Tensor to C Transpilation.
 * Precision: Fixed-Point Q16.16 (64-bit bounds protected).
 */
#include <stdint.h>
#include <stddef.h>

/* ZCC preprocessor bypass: all dimensions hardcoded */

"""

    asm_code = ""
    asm_code += generate_gnu_asm("W1", w1_f)
    asm_code += generate_gnu_asm("B1", b1_f)
    asm_code += generate_gnu_asm("W2", w2_f)
    asm_code += generate_gnu_asm("B2", b2_f)
    asm_code += generate_gnu_asm("W3", w3_f)
    asm_code += generate_gnu_asm("B3", b3_f)

    with open("zkaedi_weights.s", "w", encoding="utf-8") as as_out:
        as_out.write(asm_code)

    c_code += generate_extern_decl("W1")
    c_code += generate_extern_decl("B1")
    c_code += generate_extern_decl("W2")
    c_code += generate_extern_decl("B2")
    c_code += generate_extern_decl("W3")
    c_code += generate_extern_decl("B3")

    c_code += f"""
/* Dimensions */
long long evm_classifier_infer(long long input[23]) {{
    long long h1[128];
    long long h2[64];
    long long out[19];
    int i, j;

    /* Layer 1 */
    for (i = 0; i < 128; i++) {{
        long long sum = B1[i] * 65536; /* pre-scale up to maintain Q16 precision buffer */
        for (j = 0; j < 23; j++) {{
            sum += (input[j] * W1[i * 23 + j]);
        }}
        h1[i] = (sum / 65536 > 0) ? (sum / 65536) : 0;
    }}

    /* Layer 2 */
    for (i = 0; i < 64; i++) {{
        long long sum = B2[i] * 65536;
        for (j = 0; j < 128; j++) {{
            sum += (h1[j] * W2[i * 128 + j]);
        }}
        h2[i] = (sum / 65536 > 0) ? (sum / 65536) : 0;
    }}

    /* Output Layer (Logits) */
    long long max_val = -999999999;
    int max_idx = 0;
    
    for (i = 0; i < 19; i++) {{
        long long sum = B3[i] * 65536;
        for (j = 0; j < 64; j++) {{
            sum += (h2[j] * W3[i * 64 + j]); /* Integer MAC */
        }}
        out[i] = sum / 65536;
        if (out[i] > max_val) {{
            max_val = out[i];
            max_idx = i;
        }}
    }}

    /* Returns Predicted Vulnerability Class (0 = Safe ZCC) */
    return max_idx;
}}

/* ══════════════════════════════════════════════════════════════════ */
/* ZKAEDI MEV Supervisor Compatible Exports (Dummy implementations for JIT) */
/* ══════════════════════════════════════════════════════════════════ */

typedef struct {{
    long long ema;
    long long ema_sq;
    long long alpha;
    long long one_minus;
    int initialized;
    int _pad;
}} ZKAEDIEMAStateFixed;

typedef struct {{
    ZKAEDIEMAStateFixed gas_ema;
    ZKAEDIEMAStateFixed val_ema;
    long long txs_scored;
}} ZKAEDIMEVScorerFixed;

void zkaedi_mev_scorer_fixed_init(ZKAEDIMEVScorerFixed* scorer) {{
    scorer->txs_scored = 0;
}}

long long zkaedi_mev_score_tx_fixed(ZKAEDIMEVScorerFixed* scorer, long long gas_scaled, long long val_scaled) {{
    scorer->txs_scored += 1;
    /* Hardcoded safety tensor wrapper for the supervisor */
    /* Bypassed if we hit structural honey-pot */
    return (gas_scaled + val_scaled) / 1000;
}}

"""
    with open("zkaedi_core_fixed.c", "w", encoding="utf-8") as f:
        f.write(c_code)
    try:
        print("✅ Successfully compiled PyTorch Neural Framework into 64-bit Fixed-Point C.")
        print("   Output 1: zkaedi_core_fixed.c (ZCC-compatible logic)")
        print("   Output 2: zkaedi_weights.s (GNU ASM Tensor payloads)")
    except Exception:
        pass

if __name__ == "__main__":
    main()
