from typing import List

from ._interfaces import IDustSensor, IObstacleSensor


class ObstacleSensorSubject:
    """ObstacleSensor 폴링/통지 Subject. dev plan Phase 1 팀 A 산출물 모방."""

    def __init__(self, sensor: IObstacleSensor) -> None:
        self._sensor = sensor
        self._observers: List = []
        self._last_front = False
        self._last_left = False
        self._last_right = False

    def attach(self, observer) -> None:
        if observer not in self._observers:
            self._observers.append(observer)

    def detach(self, observer) -> None:
        if observer in self._observers:
            self._observers.remove(observer)

    def notify(self) -> None:
        for o in self._observers:
            o.onObstacleDetected(self._last_front, self._last_left, self._last_right)

    def poll(self) -> None:
        f = self._sensor.isFrontDetected()
        l = self._sensor.isLeftDetected()
        r = self._sensor.isRightDetected()
        if (f, l, r) != (self._last_front, self._last_left, self._last_right):
            self._last_front, self._last_left, self._last_right = f, l, r
            self.notify()

    def onInterrupt(self) -> None:
        f = self._sensor.isFrontDetected()
        l = self._sensor.isLeftDetected()
        r = self._sensor.isRightDetected()
        self._last_front, self._last_left, self._last_right = f, l, r
        self.notify()

    def getSensor(self) -> IObstacleSensor:
        return self._sensor


class DustSensorSubject:
    """DustSensor 폴링/통지 Subject. dev plan Phase 1 팀 B 산출물 모방."""

    def __init__(self, sensor: IDustSensor) -> None:
        self._sensor = sensor
        self._observers: List = []
        self._last_dust = False

    def attach(self, observer) -> None:
        if observer not in self._observers:
            self._observers.append(observer)

    def detach(self, observer) -> None:
        if observer in self._observers:
            self._observers.remove(observer)

    def notify(self) -> None:
        for o in self._observers:
            o.onDustDetected(self._last_dust)

    def poll(self) -> None:
        d = self._sensor.isDustDetected()
        if d != self._last_dust:
            self._last_dust = d
            self.notify()

    def getSensor(self) -> IDustSensor:
        return self._sensor
