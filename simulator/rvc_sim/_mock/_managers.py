from typing import Callable, Optional

from ._interfaces import ICleaner, IMotor
from ._strategies import IAvoidStrategy
from ._subjects import DustSensorSubject
from ._timer import Timer
from ._types import Direction, PowerLevel


class MovementManager:
    """모터 제어 + 1회 회피 결정 (dev plan AD-1).

    회피 후진 루프 / 후진 후 재확인은 본 클래스가 책임지지 않음 — 그건
    AvoidingState 쪽 오케스트레이션.
    """

    def __init__(self, motor: IMotor, avoid_strategy: IAvoidStrategy) -> None:
        self._motor = motor
        self._strategy = avoid_strategy

    def moveForward(self) -> None:
        self._motor.move(Direction.FORWARD)

    def moveBackward(self) -> None:
        self._motor.move(Direction.BACKWARD)

    def turn(self, direction: Direction) -> None:
        self._motor.move(direction)

    def stop(self) -> None:
        self._motor.move(Direction.STOP)

    def executeAvoidance(self, front: bool, left: bool, right: bool) -> None:
        direction = self._strategy.decideDirection(front, left, right)
        self._motor.move(direction)


class CleaningManager:
    """청소 제어 + 3초 PowerUp Timer (dev plan Phase 1 팀 B)."""

    POWER_UP_DURATION_MS = 3000

    def __init__(
        self,
        cleaner: ICleaner,
        dust_sensor_subject: DustSensorSubject,
        clock_fn: Optional[Callable[[], int]] = None,
    ) -> None:
        self._cleaner = cleaner
        self._dust_sub = dust_sensor_subject
        self._timer = Timer(clock_fn)
        self._timer.setDuration(self.POWER_UP_DURATION_MS)
        self._timer.setCallback(self._onTimerExpired)
        self._power = PowerLevel.OFF

    def startCleaning(self) -> None:
        self._setPower(PowerLevel.NORMAL)

    def stopCleaning(self) -> None:
        self._setPower(PowerLevel.OFF)
        self._timer.stop()

    def powerUp(self) -> None:
        self._setPower(PowerLevel.POWER_UP)
        self._timer.reset()
        self._timer.start()

    def handleDustDetected(self, detected: bool) -> None:
        if detected:
            self.powerUp()

    def update(self) -> None:
        self._timer.update()

    def getPowerLevel(self) -> PowerLevel:
        return self._power

    def _setPower(self, level: PowerLevel) -> None:
        self._power = level
        self._cleaner.setPower(level)

    def _onTimerExpired(self) -> None:
        self._dust_sub.poll()
        sensor = self._dust_sub.getSensor()
        still_dusty = sensor.isDustDetected()
        if still_dusty:
            self._timer.reset()
            self._timer.start()
        else:
            self._setPower(PowerLevel.NORMAL)
            self._timer.stop()
