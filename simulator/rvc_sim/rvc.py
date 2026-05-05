"""rvc module shim — exposes the C++ controller API.

Uses the real pybind11-built ``rvc`` module if available; otherwise falls back
to the in-repo Mock controller under ``rvc_sim._mock``.

When real bindings are built and importable, the entire ``rvc_sim/_mock/``
directory can be deleted and the fallback branch in this shim removed.
"""
try:
    from rvc import (  # type: ignore
        Direction,
        PowerLevel,
        IMotor,
        ICleaner,
        IObstacleSensor,
        IDustSensor,
        RVCController,
    )
    _BACKEND = "binding"
except ImportError:
    from ._mock.rvc_compat import (
        Direction,
        PowerLevel,
        IMotor,
        ICleaner,
        IObstacleSensor,
        IDustSensor,
        RVCController,
    )
    _BACKEND = "mock"


def backend() -> str:
    """Return the active backend name: ``"binding"`` or ``"mock"``."""
    return _BACKEND


__all__ = [
    "Direction",
    "PowerLevel",
    "IMotor",
    "ICleaner",
    "IObstacleSensor",
    "IDustSensor",
    "RVCController",
    "backend",
]
