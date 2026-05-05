"""Parametrized scenario test: every YAML in scenarios/ must pass its assertions."""
from __future__ import annotations

from pathlib import Path

import pytest

from rvc_sim.headless import run_scenario

from conftest import discover_scenarios


SCENARIO_FILES = discover_scenarios()


@pytest.mark.parametrize(
    "scenario_path",
    SCENARIO_FILES,
    ids=[p.stem for p in SCENARIO_FILES],
)
def test_scenario(scenario_path: Path) -> None:
    result, failures = run_scenario(scenario_path)
    assert not failures, (
        f"Scenario {scenario_path.name} failed:\n"
        + "\n".join(f"  - [{spec.get('kind')}] {err}" for spec, err in failures)
    )
    assert result.power_off_invoked, "powerOff was not called by the runner"
