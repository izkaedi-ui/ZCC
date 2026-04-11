import json
orig = json.load(open('zcc_ir.json'))
opt = json.load(open('zcc_ir_optimized.json'))

side_ops = {'IR_RET','IR_BR','IR_BR_IF','IR_STORE','IR_CALL','IR_ARG','IR_LABEL'}
def count_side(ir):
    n = 0
    for func in ir:
        for inst in func.get('instructions', func.get('body', [])):
            if inst['op'] in side_ops:
                n += 1
    return n

orig_side = count_side(orig)
opt_side = count_side(opt)
assert orig_side == opt_side, f'SAFETY VIOLATION: {orig_side} vs {opt_side} side-effecting ops'
print(f'Safety verified: {opt_side} side-effecting instructions preserved')
assert len(orig) == len(opt), f'Function count mismatch: {len(orig)} vs {len(opt)}'
print(f'Function count verified: {len(opt)} functions')
