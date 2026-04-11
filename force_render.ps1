$files = @("part1.c", "part2.c", "part3.c", "part4.c", "part5.c", "ir.c", "ir.h", "ir_emit_dispatch.h", "compiler_passes.c")
$dir = "H:\__DOWNLOADS\selforglinux"
foreach ($f in $files) {
    if (Test-Path "$dir\$f") {
        Add-Content -Path "$dir\$f" -Value "`n/* ZKAEDI FORCE RENDER CACHE INVALIDATION */"
    }
}
