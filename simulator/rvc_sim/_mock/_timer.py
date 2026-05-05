from typing import Callable, Optional


class Timer:
    """Mirror of ``rvc::Timer`` (include/rvc/timer.hpp).

    Clock is injected as a callable returning monotonic milliseconds. Default
    clock returns 0 — callers in tests must inject a fake clock for any
    duration-dependent behavior.
    """

    def __init__(self, clock_fn: Optional[Callable[[], int]] = None) -> None:
        self._clock_fn: Callable[[], int] = clock_fn or (lambda: 0)
        self._callback: Optional[Callable[[], None]] = None
        self._duration_ms: int = 0
        self._start_time: int = 0
        self._is_running: bool = False
        self._is_fired: bool = False

    def setDuration(self, duration_ms: int) -> None:
        self._duration_ms = duration_ms

    def setCallback(self, callback: Callable[[], None]) -> None:
        self._callback = callback

    def start(self) -> None:
        self._start_time = self._clock_fn()
        self._is_running = True
        self._is_fired = False

    def stop(self) -> None:
        self._is_running = False

    def reset(self) -> None:
        self._is_running = False
        self._is_fired = False
        self._start_time = 0

    def isRunning(self) -> bool:
        return self._is_running

    def isExpired(self) -> bool:
        if not self._is_running:
            return False
        return (self._clock_fn() - self._start_time) >= self._duration_ms

    def update(self) -> None:
        if self._is_running and not self._is_fired and self.isExpired():
            self._is_fired = True
            self._is_running = False
            if self._callback is not None:
                self._callback()
