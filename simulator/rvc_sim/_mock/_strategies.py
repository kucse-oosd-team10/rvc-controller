from abc import ABC, abstractmethod

from ._types import Direction


class IAvoidStrategy(ABC):
    @abstractmethod
    def decideDirection(self, front: bool, left: bool, right: bool) -> Direction: ...

    @abstractmethod
    def needsReverse(self, front: bool, left: bool, right: bool) -> bool: ...


class DefaultAvoidStrategy(IAvoidStrategy):
    """Left-first priority avoidance, per dev plan truth table.

    (F,F,F) -> Forward
    (T,F,?) -> Left
    (T,T,F) -> Right
    (T,T,T) -> Backward, needsReverse=True
    """

    def decideDirection(self, front: bool, left: bool, right: bool) -> Direction:
        if not front:
            return Direction.FORWARD
        if not left:
            return Direction.LEFT
        if not right:
            return Direction.RIGHT
        return Direction.BACKWARD

    def needsReverse(self, front: bool, left: bool, right: bool) -> bool:
        return front and left and right
