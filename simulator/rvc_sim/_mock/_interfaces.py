from abc import ABC, abstractmethod

from ._types import Direction, PowerLevel


class IMotor(ABC):
    @abstractmethod
    def initialize(self) -> bool: ...

    @abstractmethod
    def move(self, direction: Direction) -> None: ...


class ICleaner(ABC):
    @abstractmethod
    def initialize(self) -> bool: ...

    @abstractmethod
    def setPower(self, level: PowerLevel) -> None: ...


class IObstacleSensor(ABC):
    @abstractmethod
    def initialize(self) -> bool: ...

    @abstractmethod
    def isFrontDetected(self) -> bool: ...

    @abstractmethod
    def isLeftDetected(self) -> bool: ...

    @abstractmethod
    def isRightDetected(self) -> bool: ...


class IDustSensor(ABC):
    @abstractmethod
    def initialize(self) -> bool: ...

    @abstractmethod
    def isDustDetected(self) -> bool: ...
