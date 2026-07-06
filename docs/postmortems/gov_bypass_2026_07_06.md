# QEC-VOP Governance Incident Postmortem: Direct Push Bypass

**Incident Date**: 2026-07-06  
**Severity**: Medium  
**Status**: Closed/Resolved  

---

## 🔱 Trigger
During the rollout of **QEC-VOP v1.1 Week-2 (Forecasting & Breach Probability)**, commits were pushed directly to `main` without opening a Pull Request.

---

## 🔱 Root Cause & Invariant Violated
- **Violation**: The repository's branch protection rules permitted administrators or authenticated tokens to bypass PR constraints during direct pushes.
- **Impact**: Bypassed status check gates (`validate`), undermining the process integrity of the QEC Verification Operations Platform.

---

## 🔱 Remediation Plan
1. **Merge Policy Hardened**: Updated `docs/merge_policy.md` to formally outlaw direct pushes to `main`.
2. **Branch Protection Update**: Enforce strict status checks for all users (including administrators) and require PR workflows for all modifications.
3. **No Direct Pushes**: All subsequent features (including Week-3 and Week-4) will be submitted exclusively via branch PRs.
