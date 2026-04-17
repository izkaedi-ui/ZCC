#!/usr/bin/env python3
import os
import sys
import subprocess
import json
import urllib.request
import zipfile
import shutil

JULIET_URL = "https://samate.nist.gov/SARD/downloads/test-suites/2017-10-01-juliet-test-suite-for-c-cplusplus-v1-3.zip"
CWE_CLASSES = [
    "CWE121_Stack_Based_Buffer_Overflow",
    "CWE476_NULL_Pointer_Dereference",
    "CWE190_Integer_Overflow",
    "CWE125_Out_of_Bounds_Read",
    "CWE416_Use_After_Free",
    "CWE20_Improper_Input_Validation",
    "CWE787_Out_of_Bounds_Write"
]

REAL_CVES = [
    {
        "repo": "https://github.com/curl/curl.git",
        "name": "curl_CVE-2023-38545",
        "commit_post": "4a4b63daaa",
        "commit_pre": "4a4b63daaa^", # The parent commit
        "cwe": "CWE-787"
    },
    {
        "repo": "https://github.com/sqlite/sqlite.git",
        "name": "sqlite_CVE-2022-46908",
        "commit_post": "14ae25777bd365db720745fc98ea4decc1d2a1c0", # Approx commit for 3.40.1 shell.c fix
        "commit_pre": "14ae25777bd365db720745fc98ea4decc1d2a1c0^",
        "cwe": "CWE-125"
    }
]

def run(cmd, cwd=None):
    subprocess.run(cmd, check=True, shell=True, cwd=cwd)

def setup_directories():
    os.makedirs("cve_corpus/juliet_subset", exist_ok=True)
    for cwe in CWE_CLASSES:
        os.makedirs(f"cve_corpus/juliet_subset/{cwe}", exist_ok=True)
    os.makedirs("cve_corpus/real_cves", exist_ok=True)

def fetch_juliet():
    print("Downloading Juliet C/C++ v1.3...")
    zip_path = "juliet.zip"
    if not os.path.exists(zip_path):
        urllib.request.urlretrieve(JULIET_URL, zip_path)

    # Per-CWE extraction caps:
    #   GO CWEs: uncapped (9999)
    #   CWE-121: 10 files, but ONLY memcpy/ncat/snprintf/ncpy variants
    #            to validate extract_121_memcpy_bounds.py before full scale-out
    CWE_LIMITS = {
        "CWE416_Use_After_Free":              9999,
        "CWE476_NULL_Pointer_Dereference":    9999,
        "CWE190_Integer_Overflow":            9999,
        "CWE121_Stack_Based_Buffer_Overflow": 10,   # HOLD — memcpy validation first
        "CWE125_Out_of_Bounds_Read":          9999,
        "CWE20_Improper_Input_Validation":    9999,
        "CWE787_Out_of_Bounds_Write":         9999,
    }
    # CWE-121 must contain function-call overflow patterns, not just loop patterns
    CWE121_REQUIRE_PATTERN = ("memcpy", "ncat", "ncpy", "snprintf", "memmove")

    print("Extracting subset...")
    with zipfile.ZipFile(zip_path, 'r') as zf:
        extracted = {cwe: 0 for cwe in CWE_CLASSES}
        for info in zf.infolist():
            for cwe in CWE_CLASSES:
                limit = CWE_LIMITS.get(cwe, 20)
                if extracted[cwe] >= limit:
                    continue
                if f"testcases/{cwe}/" not in info.filename:
                    continue
                if not info.filename.endswith(".c"):
                    continue
                basename = os.path.basename(info.filename)
                # CWE-121: only pull memcpy/ncat-style variants for memcpy validation sprint
                if cwe == "CWE121_Stack_Based_Buffer_Overflow":
                    if not any(p in basename for p in CWE121_REQUIRE_PATTERN):
                        continue
                out_path = f"cve_corpus/juliet_subset/{cwe}/{basename}"
                with open(out_path, "wb") as f:
                    f.write(zf.read(info.filename))
                extracted[cwe] += 1
    print(f"Juliet extraction complete: {extracted}")

def fetch_real_cves():
    print("Fetching real CVEs...")
    for cve in REAL_CVES:
        cve_dir = f"cve_corpus/real_cves/{cve['name']}"
        os.makedirs(cve_dir, exist_ok=True)
        
        with open(f"{cve_dir}/metadata.json", "w") as f:
            json.dump({
                "cwe": cve["cwe"],
                "repo": cve["repo"],
                "post": cve["commit_post"],
                "pre": cve["commit_pre"]
            }, f, indent=4)
            
        repo_name = cve["repo"].split("/")[-1].replace(".git", "")
        repo_owner = cve["repo"].split("/")[-2]
        
        # Download diff
        print(f"Downloading diff for {cve['name']}...")
        diff_url = f"https://github.com/{repo_owner}/{repo_name}/commit/{cve['commit_post']}.diff"
        urllib.request.urlretrieve(diff_url, f"{cve_dir}/patch.diff")
        
        # Download full tree for pre and post. For simplicity, just get the zip archive
        for phase, commit in [("pre_patch", cve['commit_pre']), ("post_patch", cve['commit_post'])]:
            print(f"Downloading {phase} for {cve['name']}...")
            zip_url = f"https://github.com/{repo_owner}/{repo_name}/archive/{commit}.zip"
            zip_path = f"{cve_dir}/{phase}.zip"
            try:
                urllib.request.urlretrieve(zip_url, zip_path)
                with zipfile.ZipFile(zip_path, 'r') as zf:
                    zf.extractall(f"{cve_dir}/{phase}_tmp")
                os.remove(zip_path)
                # The extracted folder is something like "curl-4a4b63daaa"
                extracted_dir = os.listdir(f"{cve_dir}/{phase}_tmp")[0]
                shutil.move(f"{cve_dir}/{phase}_tmp/{extracted_dir}", f"{cve_dir}/{phase}")
                os.rmdir(f"{cve_dir}/{phase}_tmp")
            except Exception as e:
                print(f"Failed to fetch {phase}: {e}")
                
        print(f"Finished {cve['name']}")

if __name__ == "__main__":
    setup_directories()
    fetch_juliet()
    fetch_real_cves()
