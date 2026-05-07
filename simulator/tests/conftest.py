"""Pytest configuration shared across simulator tests."""
from __future__ import annotations

import sys
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]
SCENARIOS_DIR = Path(__file__).resolve().parents[1] / "scenarios"


def _ensure_binding_on_path() -> None:
    try:
        import rvc  # noqa: F401
        return
    except ImportError:
        pass
    for candidate in sorted(REPO_ROOT.glob("build/*/bindings")):
        if any(candidate.glob("rvc*.so")):
            sys.path.insert(0, str(candidate))
            return


_ensure_binding_on_path()


def discover_scenarios() -> list[Path]:
    return sorted(SCENARIOS_DIR.glob("*.yaml"))


@pytest.fixture(scope="session")
def scenarios_dir() -> Path:
    return SCENARIOS_DIR
