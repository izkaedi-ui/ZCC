#!/bin/bash
ZCC="../zcc"
DUMP_DIR="cve_ir_dumps"

echo "Extracting IR for curl CVE-2023-38545 (lib/socks.c)"
for phase in "pre_patch" "post_patch"; do
    cve_dir="real_cves/curl_CVE-2023-38545/${phase}"
    file="${cve_dir}/lib/socks.c"
    
    gcc -E -w -I. -I"${cve_dir}/include" -I"${cve_dir}/lib" -Dsread=read -Dswrite=write -include tmp_headers.h "$file" > "tmp_${phase}.c" 2>/dev/null
    ZCC_EMIT_IR=1 $ZCC "tmp_${phase}.c" -o /tmp/null.s > "${DUMP_DIR}/curl_CVE-2023-38545_${phase}.ir"
    
    echo "Generated for ${phase}: $(wc -l < ${DUMP_DIR}/curl_CVE-2023-38545_${phase}.ir) lines of IR"
done
