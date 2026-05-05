"""Headless 시뮬레이션 러너 + Snapshot + FakeClock."""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, List, Optional, Protocol

from .adapters import SimCleaner, SimDustSensor, SimMotor, SimObstacleSensor
from .robot import Robot
from .rvc import Direction, PowerLevel, RVCController, backend
from .scenario import CleaningRates, Scenario
from .types import Heading
from .world import World


class FakeClock:
    """Monotonic millisecond clock under runner's control."""

    def __init__(self, start_ms: int = 0) -> None:
        self.now_ms = start_ms

    def __call__(self) -> int:
        return self.now_ms

    def advance(self, ms: int) -> None:
        self.now_ms += ms


@dataclass
class Snapshot:
    tick: int
    clock_ms: int
    robot_x: int
    robot_y: int
    robot_heading: str
    state_name: str
    cleaner_power: PowerLevel
    motor_history_len: int
    last_motor_command: Optional[Direction]
    dust_total: float
    collision_count: int


@dataclass
class RunResult:
    scenario: Scenario
    snapshots: List[Snapshot]
    motor_history: List[Direction]
    cleaner_history: List[PowerLevel]
    collisions: List[tuple]
    initial_dust: float
    final_dust: float
    state_after_power_off: str
    power_off_invoked: bool


class GuiRenderer(Protocol):
    def render(self, runner: "SimulationRunner") -> bool: ...


def cleaning_rate_for(power: PowerLevel, rates: CleaningRates) -> float:
    return {
        PowerLevel.OFF: rates.off,
        PowerLevel.NORMAL: rates.normal,
        PowerLevel.POWER_UP: rates.power_up,
    }[power]


def build_world(scenario: Scenario) -> World:
    spec = scenario.world
    return World(
        width=spec.width,
        height=spec.height,
        obstacles=set(spec.obstacles),
        dust=dict(spec.dust),
        dust_threshold=spec.dust_threshold,
    )


class SimulationRunner:
    def __init__(
        self,
        scenario: Scenario,
        *,
        gui: Optional[GuiRenderer] = None,
        tick_duration_ms_override: Optional[int] = None,
    ) -> None:
        self.scenario = scenario
        self.tick_duration_ms = tick_duration_ms_override or scenario.tick_duration_ms

        self.world = build_world(scenario)
        self.robot = Robot(
            scenario.robot.start[0],
            scenario.robot.start[1],
            Heading.from_label(scenario.robot.heading),
        )
        self.clock = FakeClock()

        self.motor = SimMotor(
            self.robot, self.world, init_fail_count=scenario.init_failures.motor
        )
        self.cleaner = SimCleaner(init_fail_count=scenario.init_failures.cleaner)
        self.obs_sensor = SimObstacleSensor(
            self.world, self.robot, init_fail_count=scenario.init_failures.obstacle_sensor
        )
        self.dust_sensor = SimDustSensor(
            self.world, self.robot, init_fail_count=scenario.init_failures.dust_sensor
        )

        self.controller = self._build_controller()

        self.tick_count = 0
        self.snapshots: List[Snapshot] = []
        self.gui = gui
        self._initial_dust = self.world.total_dust()

    def _build_controller(self) -> RVCController:
        kwargs: dict = {}
        if backend() == "mock":
            kwargs["clock"] = self.clock
        return RVCController(
            self.motor, self.cleaner, self.obs_sensor, self.dust_sensor, **kwargs
        )

    def _snapshot(self) -> Snapshot:
        last_cmd = self.motor.history[-1] if self.motor.history else None
        return Snapshot(
            tick=self.tick_count,
            clock_ms=self.clock.now_ms,
            robot_x=self.robot.x,
            robot_y=self.robot.y,
            robot_heading=self.robot.heading.name,
            state_name=self.controller.current_state_name,
            cleaner_power=self.cleaner.current_power,
            motor_history_len=len(self.motor.history),
            last_motor_command=last_cmd,
            dust_total=self.world.total_dust(),
            collision_count=len(self.world.collisions),
        )

    def run(self) -> RunResult:
        self.controller.powerOn()
        self.snapshots.append(self._snapshot())

        power_off_invoked = False
        for _ in range(self.scenario.max_ticks):
            if (
                self.scenario.power_off_at_tick is not None
                and self.tick_count == self.scenario.power_off_at_tick
            ):
                self.controller.powerOff()
                power_off_invoked = True
                break

            self.clock.advance(self.tick_duration_ms)
            self.controller.tick()
            self.motor.step_continuous()

            rate = cleaning_rate_for(self.cleaner.current_power, self.scenario.cleaning_rates)
            if rate > 0:
                self.world.collect_dust(self.robot.pose, rate)

            self.tick_count += 1
            self.snapshots.append(self._snapshot())

            if self.gui is not None:
                if not self.gui.render(self):
                    break
        else:
            self.controller.powerOff()
            power_off_invoked = True

        if not power_off_invoked:
            self.controller.powerOff()
            power_off_invoked = True

        return RunResult(
            scenario=self.scenario,
            snapshots=list(self.snapshots),
            motor_history=list(self.motor.history),
            cleaner_history=list(self.cleaner.power_history),
            collisions=list(self.world.collisions),
            initial_dust=self._initial_dust,
            final_dust=self.world.total_dust(),
            state_after_power_off=self.controller.current_state_name,
            power_off_invoked=power_off_invoked,
        )
