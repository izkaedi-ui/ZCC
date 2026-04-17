#!/usr/bin/env python3
import json
import sys
import copy

def is_power_of_2(n):
    if not isinstance(n, int) or n <= 0: return False
    return (n & (n - 1)) == 0

def log2_exact(n):
    res = 0
    while n > 1:
        n >>= 1
        res += 1
    return res

def build_cfg(instrs):
    # Basic Block Identification
    blocks = []
    current_block = []
    
    for i, ins in enumerate(instrs):
        op = ins.get('op')
        
        # LABEL or Function entry ends current block and starts new
        if op == 'LABEL':
            if current_block:
                blocks.append(current_block)
            current_block = [ins]
        else:
            current_block.append(ins)
            
        # Branch/Ret ends current block
        if op in ('BR', 'BR_IF', 'RET'):
            blocks.append(current_block)
            current_block = []
            
    if current_block:
        blocks.append(current_block)
        
    return blocks

def constant_fold(blocks):
    folded = 0
    # Map from IR tmp to constant value
    constants = {}
    
    for block in blocks:
        for ins in block:
            op = ins.get('op')
            dst = ins.get('dst')
            
            if op == 'CONST':
                val = ins.get('value')
                if isinstance(val, int):
                    constants[dst] = val
            
            elif op in ('ADD', 'SUB', 'MUL', 'DIV', 'MOD', 'AND', 'OR', 'XOR', 'SHL', 'SHR', 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE'):
                src1 = ins.get('src1')
                src2 = ins.get('src2')
                
                v1 = constants.get(src1)
                v2 = constants.get(src2)
                
                if v1 is not None and v2 is not None:
                    try:
                        res = None
                        if op == 'ADD': res = v1 + v2
                        elif op == 'SUB': res = v1 - v2
                        elif op == 'MUL': res = v1 * v2
                        elif op == 'DIV' and v2 != 0: res = v1 // v2
                        elif op == 'MOD' and v2 != 0: res = v1 % v2
                        elif op == 'AND': res = v1 & v2
                        elif op == 'OR': res = v1 | v2
                        elif op == 'XOR': res = v1 ^ v2
                        elif op == 'SHL': res = v1 << v2
                        elif op == 'SHR': res = v1 >> v2
                        elif op == 'EQ': res = 1 if v1 == v2 else 0
                        elif op == 'NE': res = 1 if v1 != v2 else 0
                        elif op == 'LT': res = 1 if v1 < v2 else 0
                        elif op == 'LE': res = 1 if v1 <= v2 else 0
                        elif op == 'GT': res = 1 if v1 > v2 else 0
                        elif op == 'GE': res = 1 if v1 >= v2 else 0
                        
                        if res is not None:
                            ins['op'] = 'CONST'
                            ins['value'] = res
                            if 'src1' in ins: del ins['src1']
                            if 'src2' in ins: del ins['src2']
                            constants[dst] = res
                            folded += 1
                    except Exception:
                        pass
    return folded


def copy_propagation(blocks):
    propagated = 0
    copy_map = {}
    
    for block in blocks:
        for ins in block:
            # Resolve chains
            for key in ('src', 'src1', 'src2', 'lhs', 'rhs', 'cond', 'addr', 'arg'):
                s = ins.get(key)
                if s and s in copy_map:
                    # traverse to root
                    v = s
                    while v in copy_map:
                        v = copy_map[v]
                    ins[key] = v
                    propagated += 1
            
            if 'args' in ins:
                new_args = []
                for a in ins['args']:
                    if a in copy_map:
                        v = a
                        while v in copy_map:
                            v = copy_map[v]
                        new_args.append(v)
                        propagated += 1
                    else:
                        new_args.append(a)
                ins['args'] = new_args
                
            op = ins.get('op')
            if op == 'COPY' and 'src' in ins:
                copy_map[ins['dst']] = ins['src']
                
    return propagated

def strength_reduction(blocks):
    reduced = 0
    constants = {}
    
    for block in blocks:
        for ins in block:
            op = ins.get('op')
            dst = ins.get('dst')
            if op == 'CONST':
                val = ins.get('value')
                if isinstance(val, int):
                    constants[dst] = val
                    
            elif op == 'MUL':
                s1 = ins.get('src1')
                s2 = ins.get('src2')
                v1 = constants.get(s1)
                v2 = constants.get(s2)
                
                if v2 == 2:
                    ins['op'] = 'ADD'
                    ins['src2'] = s1
                    reduced += 1
                elif v1 == 2:
                    ins['op'] = 'ADD'
                    ins['src1'] = s2
                    reduced += 1
                elif v2 is not None and is_power_of_2(v2):
                    ins['op'] = 'SHL'
                    # Inject a new CONST instruction for the shift amount right before this one?
                    # The prompt says: MUL x, power_of_2 -> SHL x, n
                    # ZCC's SHL takes a register. Let's just create a new op
                    ins['op_note'] = 'strength_reduced_shl'
                    ins['shamt'] = log2_exact(v2)
                    reduced += 1
            elif op == 'DIV':
                s2 = ins.get('src2')
                v2 = constants.get(s2)
                if v2 is not None and is_power_of_2(v2):
                    ins['op'] = 'SHR'
                    ins['op_note'] = 'strength_reduced_shr'
                    ins['shamt'] = log2_exact(v2)
                    reduced += 1
                    
    return reduced

def run_dce(blocks):
    used_regs = set()
    critical_ops = {'STORE', 'CALL', 'RET', 'BR', 'BR_IF'}
    
    def extract_uses(ins):
        uses = set()
        for k in ('src', 'src1', 'src2', 'lhs', 'rhs', 'cond', 'addr', 'arg'):
            if ins.get(k): uses.add(ins[k])
        if 'args' in ins:
            uses.update(ins['args'])
        return uses
    
    worklist = []
    
    # 1. Gather all critical instructions and initial uses
    ins_map = {}
    for block in blocks:
        for ins in block:
            dst = ins.get('dst')
            if dst:
                ins_map[dst] = ins
            if ins.get('op') in critical_ops:
                uses = extract_uses(ins)
                used_regs.update(uses)
                worklist.extend(uses)
                
    # 2. Propagate backwards
    while worklist:
        v = worklist.pop()
        if v in ins_map:
            defin = ins_map[v]
            uses = extract_uses(defin)
            for u in uses:
                if u not in used_regs:
                    used_regs.add(u)
                    worklist.append(u)
                    
    # 3. Sweep
    swept = 0
    new_blocks = []
    for block in blocks:
        new_b = []
        for ins in block:
            dst = ins.get('dst')
            if dst and dst not in used_regs and ins.get('op') not in critical_ops:
                swept += 1
            else:
                new_b.append(ins)
        if new_b:
            new_blocks.append(new_b)
            
    return new_blocks, swept

def process_file(in_path, out_path):
    with open(in_path, 'r') as f:
        data = json.load(f)
        
    start_count = sum(len(f['instrs']) for f in data)
    print(f"Initial Instructions: {start_count}")
        
    total_folded = 0
    total_copied = 0
    total_swept = 0
    total_reduced = 0
    
    for fn in data:
        instrs = fn['instrs']
        blocks = build_cfg(instrs)
        
        # Iterative pass
        while True:
            f = constant_fold(blocks)
            c = copy_propagation(blocks)
            blocks, d = run_dce(blocks)
            r = strength_reduction(blocks)
            
            total_folded += f
            total_copied += c
            total_swept += d
            total_reduced += r
            
            if f + c + d + r == 0:
                break
                
        # Flatten
        flat = []
        for b in blocks:
            flat.extend(b)
        fn['instrs'] = flat
        
    end_count = sum(len(f['instrs']) for f in data)
    print(f"Final Instructions:   {end_count} (Saved {start_count - end_count})")
    print(f"Breakdown:")
    print(f"  Constant Folded:  {total_folded}")
    print(f"  Copy Propagated:  {total_copied}")
    print(f"  Strength Reduced: {total_reduced}")
    print(f"  Dead Code Swept:  {total_swept}")
    
    with open(out_path, 'w') as f:
        json.dump(data, f, indent=2)

if __name__ == '__main__':
    in_file = sys.argv[1] if len(sys.argv) > 1 else 'zcc_ir.json'
    out_file = sys.argv[2] if len(sys.argv) > 2 else 'zcc_ir_opt_v2.json'
    process_file(in_file, out_file)
