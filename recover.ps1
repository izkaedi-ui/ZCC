$target_files = @("part1.c", "part2.c", "part3.c", "part4.c", "part5.c", "ir_emit_dispatch.h", "compiler_passes.c", "ir.c")
$selforg_dir = "H:\__DOWNLOADS\selforglinux"
$downloads_dir = "H:\__DOWNLOADS"

foreach ($file in $target_files) {
    $baseName = $file.Split('.')[0]
    $ext = $file.Split('.')[-1]
    
    $searchPattern = "$baseName*.$ext"
    
    $candidates = Get-ChildItem -Path $downloads_dir -Filter $searchPattern -File -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.DirectoryName -notmatch "selforglinux" -and $_.Length -gt 0 } |
        Sort-Object LastWriteTime -Descending

    if ($candidates) {
        $best = $candidates[0]
        Copy-Item -Path $best.FullName -Destination "$selforg_dir\$file" -Force
        Write-Host "Recovered $file from $($best.FullName) (Size: $($best.Length))"
    } else {
        Write-Host "WARNING: Could not find valid backup for $file"
    }
}
