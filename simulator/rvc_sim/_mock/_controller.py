from typing import Callable, Optional

from ._interfaces import ICleaner, IDustSensor, IMotor, IObstacleSensor
from ._managers import CleaningManager, MovementManager
from ._states import ErrorState, InitializingState, IRVCState, OffState
from ._strategies import DefaultAvoidStrategy
from ._subjects import DustSensorSubject, ObstacleSensorSubject


class RVCController:
    """Mock RVCController matching the assumed Phase 3 API.

    Constructor follows the dev plan:
        RVCController(IMotor*, ICleaner*, IObstacleSensor*, IDustSensor*)

    Mock-only kwargs (not part of the real binding signature):
        clock — callable returning monotonic milliseconds (int) for the
                internal Timer's fake clock.
    """

    def __init__(
        self,
        motor: IMotor,
        cleaner: ICleaner,
        obstacle_sensor: IObstacleSensor,
        dust_sensor: IDustSensor,
        *,
        clock: Optional[Callable[[], int]] = None,
    ) -> None:
        self._motor = motor
        self._cleaner = cleaner
        self._obstacle_sensor = obstacle_sensor
        self._dust_sensor = dust_sensor

        self._avoid_strategy = DefaultAvoidStrategy()
        self._movement_mgr = MovementManager(motor, self._avoid_strategy)
        self._obstacle_sub = ObstacleSensorSubject(obstacle_sensor)
        self._dust_sub = DustSensorSubject(dust_sensor)
        self._cleaning_mgr = CleaningManager(cleaner, self._dust_sub, clock_fn=clock)

        self._current_state: Optional[IRVCState] = OffState()

    # --- Observer interface (called by Subjects on notify) ---

    def onObstacleDetected(self, front: bool, left: bool, right: bool) -> None:
        if self._current_state is not None:
            self._current_state.handleObstacle(self, front, left, right)

    def onDustDetected(self, detected: bool) -> None:
        if self._current_state is not None:
            self._current_state.handleDust(self, detected)

    # --- Public lifecycle (AD-3, AD-4) ---

    def powerOn(self) -> None:
        if not isinstance(self._current_state, OffState):
            return
        self.setState(InitializingState())
        if not isinstance(self._current_state, ErrorState):
            self._obstacle_sub.attach(self)
            self._dust_sub.attach(self)
            print("Ready")

    def powerOff(self) -> None:
        if self._current_state is not None:
            self._current_state.handlePowerOff(self)
        self._obstacle_sub.detach(self)
        self._dust_sub.detach(self)
        print("Off")

    def tick(self) -> None:
        if self._current_state is not None:
            self._current_state.update(self)
        self._obstacle_sub.poll()
        self._dust_sub.poll()
        self._cleaning_mgr.update()

    def setState(self, state: IRVCState) -> None:
        if self._current_state is not None:
            self._current_state.onExit(self)
        self._current_state = state
        if self._current_state is not None:
            self._current_state.onEnter(self)

    # --- Getters consumed by State classes ---

    def getMovementManager(self) -> MovementManager:
        return self._movement_mgr

    def getCleaningManager(self) -> CleaningManager:
        return self._cleaning_mgr

    def getObstacleSensorSubject(self) -> ObstacleSensorSubject:
        return self._obstacle_sub

    def getDustSensorSubject(self) -> DustSensorSubject:
        return self._dust_sub

    def getObstacleSensor(self) -> IObstacleSensor:
        return self._obstacle_sensor

    def getDustSensor(self) -> IDustSensor:
        return self._dust_sensor

    def getMotor(self) -> IMotor:
        return self._motor

    def getCleaner(self) -> ICleaner:
        return self._cleaner

    def getAvoidStrategy(self) -> DefaultAvoidStrategy:
        return self._avoid_strategy

    @property
    def current_state_name(self) -> str:
        return self._current_state.name if self._current_state is not None else "None"
