from abc import ABC, abstractmethod

from ._types import Direction, PowerLevel


class IRVCState(ABC):
    name: str = "Unknown"

    @abstractmethod
    def onEnter(self, ctx) -> None: ...

    @abstractmethod
    def onExit(self, ctx) -> None: ...

    @abstractmethod
    def handleObstacle(self, ctx, front: bool, left: bool, right: bool) -> None: ...

    @abstractmethod
    def handleDust(self, ctx, detected: bool) -> None: ...

    @abstractmethod
    def handlePowerOff(self, ctx) -> None: ...

    def update(self, ctx) -> None:
        """Per-tick callback. States with multi-tick logic override."""
        return None


class OffState(IRVCState):
    name = "Off"

    def onEnter(self, ctx): pass
    def onExit(self, ctx): pass
    def handleObstacle(self, ctx, front, left, right): pass
    def handleDust(self, ctx, detected): pass
    def handlePowerOff(self, ctx): pass


class InitializingState(IRVCState):
    name = "Initializing"
    MAX_RETRY = 3

    def __init__(self) -> None:
        self.retry_count = 0

    def onEnter(self, ctx):
        for _ in range(self.MAX_RETRY):
            if self._tryInit(ctx):
                ctx.setState(CleaningState())
                return
            self.retry_count += 1
        ctx.setState(ErrorState())

    def _tryInit(self, ctx) -> bool:
        return (
            ctx.getObstacleSensor().initialize()
            and ctx.getDustSensor().initialize()
            and ctx.getMotor().initialize()
            and ctx.getCleaner().initialize()
        )

    def onExit(self, ctx): pass
    def handleObstacle(self, ctx, front, left, right): pass
    def handleDust(self, ctx, detected): pass
    def handlePowerOff(self, ctx):
        ctx.setState(OffState())


class CleaningState(IRVCState):
    name = "Cleaning"

    def onEnter(self, ctx):
        ctx.getMovementManager().moveForward()
        ctx.getCleaningManager().startCleaning()

    def onExit(self, ctx): pass

    def handleObstacle(self, ctx, front, left, right):
        if not front:
            return
        ctx.getCleaningManager().stopCleaning()
        ctx.getMovementManager().stop()
        ctx.setState(AvoidingState(front, left, right))

    def handleDust(self, ctx, detected):
        ctx.getCleaningManager().handleDustDetected(detected)

    def handlePowerOff(self, ctx):
        ctx.getCleaningManager().stopCleaning()
        ctx.getMovementManager().stop()
        ctx.setState(OffState())


class AvoidingState(IRVCState):
    """다중 tick 회피 흐름 (phase 기반).

    PHASE_STOP    — 정지 상태에서 센서 재확인. 전방이 비었으면 Cleaning 복귀.
                    (T,T,T)면 PHASE_BACKING 으로, 그 외엔 1회 회전 후 PHASE_TURNING.
    PHASE_BACKING — 좌/우 둘 다 막혀 있는 동안 매 tick 1셀씩 후진. 풀리면 정지 후 PHASE_STOP.
    PHASE_TURNING — 회전 명령은 이미 이전 tick 에 내려졌고, 이번 tick 은 회전 동작이 끝나길
                    기다리는 단계. 다음 tick 에 Cleaning 으로 전이.
    """

    name = "Avoiding"
    PHASE_STOP = 0
    PHASE_BACKING = 1
    PHASE_TURNING = 2

    def __init__(self, front: bool, left: bool, right: bool) -> None:
        self._front = front
        self._left = left
        self._right = right
        self._phase = self.PHASE_STOP

    def onEnter(self, ctx): pass
    def onExit(self, ctx): pass

    def update(self, ctx):
        sensor = ctx.getObstacleSensor()
        mm = ctx.getMovementManager()
        strategy = ctx.getAvoidStrategy()

        if self._phase == self.PHASE_STOP:
            f = sensor.isFrontDetected()
            l = sensor.isLeftDetected()
            r = sensor.isRightDetected()
            if not f:
                ctx.setState(CleaningState())
                return
            if strategy.needsReverse(f, l, r):
                mm.moveBackward()
                self._phase = self.PHASE_BACKING
            else:
                mm.executeAvoidance(f, l, r)
                self._phase = self.PHASE_TURNING
        elif self._phase == self.PHASE_BACKING:
            l = sensor.isLeftDetected()
            r = sensor.isRightDetected()
            if l and r:
                mm.moveBackward()
            else:
                mm.stop()
                self._phase = self.PHASE_STOP
        elif self._phase == self.PHASE_TURNING:
            ctx.setState(CleaningState())

    def handleObstacle(self, ctx, front, left, right): pass
    def handleDust(self, ctx, detected): pass

    def handlePowerOff(self, ctx):
        ctx.getMovementManager().stop()
        ctx.getCleaningManager().stopCleaning()
        ctx.setState(OffState())


class ErrorState(IRVCState):
    name = "Error"

    def onEnter(self, ctx):
        print("Error: Init Failed")
        try:
            ctx.getMotor().move(Direction.STOP)
            ctx.getCleaner().setPower(PowerLevel.OFF)
        except Exception:
            pass

    def onExit(self, ctx): pass
    def handleObstacle(self, ctx, front, left, right): pass
    def handleDust(self, ctx, detected): pass
    def handlePowerOff(self, ctx):
        ctx.setState(OffState())
