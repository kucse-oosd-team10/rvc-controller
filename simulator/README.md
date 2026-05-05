# RVC Simulator

RVC 컨트롤러(C++) 의 GUI 시뮬레이터 + 시나리오 기반 시스템 테스트 하네스.

## 개요

- **GUI**: Pygame 기반 그리드/로봇 시각화
- **Headless**: CI 환경(`SDL_VIDEODRIVER=dummy`)에서 자동 동작
- **시나리오**: YAML 정의 + assertion 엔진 + pytest 통합

## 백엔드

런타임에 두 가지 컨트롤러 백엔드 중 하나를 선택:

1. **`rvc` (pybind11 binding)** — Phase 3 완료 후 활성화. `import rvc` 성공 시 자동 사용.
2. **`_mock` (Python Mock)** — 백엔드 1번이 없을 때 자동 폴백. `simulator/rvc_sim/_mock/` 하위에 격리. 실제 binding 도입 후 `_mock/` 디렉토리 통째 삭제로 정리.

`rvc_sim/rvc.py` 가 위 분기를 담당. 시뮬레이터 다른 모듈은 `from . import rvc` 만 사용.

## 설치

```bash
pip install -e simulator/
```

## 실행

```bash
# Headless
rvc-sim run simulator/scenarios/01_basic_cleaning.yaml

# GUI
rvc-sim run simulator/scenarios/01_basic_cleaning.yaml --gui

# pytest (모든 시나리오)
pytest simulator/tests/
```
