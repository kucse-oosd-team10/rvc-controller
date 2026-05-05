from enum import Enum, auto


class Direction(Enum):
    FORWARD = auto()
    BACKWARD = auto()
    LEFT = auto()
    RIGHT = auto()
    STOP = auto()


class PowerLevel(Enum):
    OFF = auto()
    NORMAL = auto()
    POWER_UP = auto()
