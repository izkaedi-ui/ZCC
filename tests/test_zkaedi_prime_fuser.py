"""Tests for Zkaedi Prime Fuser (Hamiltonian backend selection and fuse API)."""
import pytest
import numpy as np
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from zkaedi_prime_fuser import ZkaediPrimeFuser, FuseResult


class TestFuseResult:
    def test_dataclass_fields(self):
        r = FuseResult(True, "/out", 1.0, "gcc_perf", -0.5, "ok")
        assert r.success is True
        assert r.output_path == "/out"
        assert r.time_taken == 1.0
        assert r.backend_used == "gcc_perf"
        assert r.final_energy == -0.5
        assert r.message == "ok"


class TestZkaediPrimeFuser:
    def test_init_default_H(self):
        f = ZkaediPrimeFuser()
        assert f.H.shape == (8,)
        assert np.allclose(f.H, 0.5)
        assert len(f.backends) == 8

    def test_init_tuning_params(self):
        f = ZkaediPrimeFuser(eta=0.1, gamma=0.2, beta=0.05, sigma=0.02)
        assert f.eta == 0.1
        assert f.gamma == 0.2
        assert f.beta == 0.05
        assert f.sigma == 0.02

    def test_evolve_hamiltonian_shape_and_stability(self):
        f = ZkaediPrimeFuser()
        np.random.seed(42)
        H = f._evolve_hamiltonian(100, safety_bias=0.7, perf_bias=0.8, steps=50)
        assert H.shape == (8,)
        assert np.all(np.isfinite(H))
        assert np.max(np.abs(H)) < 100  # no blow-up

    def test_evolve_hamiltonian_updates_self_H(self):
        f = ZkaediPrimeFuser()
        np.random.seed(123)
        H0 = f.H.copy()
        f._evolve_hamiltonian(200, steps=10)
        assert not np.allclose(f.H, H0)

    def test_choose_backend_returns_valid_backend(self):
        f = ZkaediPrimeFuser()
        np.random.seed(1)
        backend = f._choose_backend(100, steps=20)
        assert backend in f.backends

    def test_fuse_mode_safety_returns_result(self, tmp_path):
        f = ZkaediPrimeFuser()
        out = tmp_path / "x.py"
        res = f.fuse("print(1)", filename=str(out), mode="safety")
        assert isinstance(res, FuseResult)
        assert res.backend_used == "rust_safe"
        assert res.output_path == "target/release/libfused.so"

    def test_fuse_mode_script_returns_result(self, tmp_path):
        f = ZkaediPrimeFuser()
        out = tmp_path / "y.py"
        res = f.fuse("x=1", filename=str(out), mode="script")
        assert isinstance(res, FuseResult)
        assert res.backend_used == "uv_script"

    def test_fuse_steps_override(self, tmp_path):
        f = ZkaediPrimeFuser()
        out = tmp_path / "z.py"
        res = f.fuse("pass", filename=str(out), mode="auto", steps=5)
        assert isinstance(res, FuseResult)
        assert res.time_taken >= 0

    def test_fuse_safety_bias_perf_bias_override(self, tmp_path):
        f = ZkaediPrimeFuser()
        out = tmp_path / "w.py"
        res = f.fuse("pass", filename=str(out), mode="auto", steps=3,
                     safety_bias=0.9, perf_bias=0.5)
        assert isinstance(res, FuseResult)
