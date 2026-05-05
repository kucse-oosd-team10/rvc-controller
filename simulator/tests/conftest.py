"""Pytest configuration shared across simulator tests."""
from __future__ import annotations

from pathlib import Path

import pytest

SCENARIOS_DIR = Path(__file__).resolve().parents[1] / "scenarios"


def discover_scenarios() -> list[Path]:
    return sorted(SCENARIOS_DIR.glob("*.yaml"))


@pytest.fixture(scope="session")
def scenarios_dir() -> Path:
    return SCENARIOS_DIR
