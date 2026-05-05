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
    """회피 흐름 오케스트레이션 (dev plan AD-1).

    1. needsReverse(f,l,r) (즉 (T,T,T)) 인 경우 좌/우 중 하나가 풀릴 때까지 후진 반복
    2. 후진 종료 후 모든 센서 재확인
    3. MM.executeAvoidance 로 1회 결정 + 모터 호출
    4. setState(CleaningState)
    """

    name = "Avoiding"
    MAX_BACKWARD_LOOPS = 64

    def __init__(self, front: bool, left: bool, right: bool) -> None:
        self._front = front
        self._left = left
        self._right = right

    def onEnter(self, ctx):
        mm = ctx.getMovementManager()
        sensor = ctx.getObstacleSensor()

        f, l, r = self._front, self._left, self._right

        if f and l and r:
            for _ in range(self.MAX_BACKWARD_LOOPS):
                mm.moveBackward()
                l = sensor.isLeftDetected()
                r = sensor.isRightDetected()
                if not l or not r:
                    break

        f = sensor.isFrontDetected()
        l = sensor.isLeftDetected()
        r = sensor.isRightDetected()

        mm.executeAvoidance(f, l, r)

        ctx.setState(CleaningState())

    def onExit(self, ctx): pass

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
