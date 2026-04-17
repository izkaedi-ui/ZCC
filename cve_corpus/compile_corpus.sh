#!/bin/bash
# compile_corpus.sh — Batch compile Juliet + real CVEs through ZCC to dump raw IR

ZCC="../zcc.exe"
if [ ! -x "$ZCC" ]; then
    echo "Error: zcc.exe not found at $ZCC"
    exit 1
fi

DUMP_DIR="ir_dumps"
mkdir -p "$DUMP_DIR"

echo "Compiling Juliet subset..."
for cwe_dir in juliet_subset/*/; do
    if [ ! -d "$cwe_dir" ]; then continue; fi
    base_cwe=$(basename "$cwe_dir")
    
    for file in "$cwe_dir"*.c; do
        if [ ! -f "$file" ]; then continue; fi
        base_file=$(basename "$file" .c)
        
        echo " -> $base_file"
        # Preprocess with gcc to resolve #includes and mock standard SARD macros
        gcc -E -nostdinc -D_WIN32 -DINCLUDEMAIN=1 -I. "$file" > "tmp_pp.c" 2>/dev/null
        ZCC_EMIT_IR=1 IR_TELEMETRY_OUT=/dev/null $ZCC "tmp_pp.c" -o /tmp/null.s > "$DUMP_DIR/${base_cwe}_${base_file}.ir" 2>&1
        rm -f tmp_pp.c
    done
done

echo "Compiling Real CVEs (pre/post patches)..."
for cve_dir in real_cves/*/; do
    if [ ! -d "$cve_dir" ]; then continue; fi
    base_cve=$(basename "$cve_dir")
    
    for phase in "pre_patch" "post_patch"; do
        if [ -d "${cve_dir}${phase}" ]; then
            for file in "${cve_dir}${phase}"/*.c; do
                if [ ! -f "$file" ]; then continue; fi
                base_file=$(basename "$file" .c)
                
                echo " -> $base_cve ($phase) - $base_file"
                gcc -E -I. -I"${cve_dir}${phase}/include" -I"${cve_dir}${phase}/lib" "$file" > "tmp_pp.c" 2>/dev/null
                ZCC_EMIT_IR=1 IR_TELEMETRY_OUT=/dev/null $ZCC "tmp_pp.c" -o /tmp/null.s > "$DUMP_DIR/${base_cve}_${phase}_${base_file}.ir" 2>&1
                rm -f tmp_pp.c
            done
        fi
    done
done

echo "Done dumping IR."
