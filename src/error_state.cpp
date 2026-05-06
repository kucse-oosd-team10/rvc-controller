#include "rvc/error_state.hpp"

#include "rvc/rvc_controller.hpp"

namespace rvc {
void ErrorState::onEnter(RVCController& ctx) {
    std::cout << "[ErrorState] Error: Init 실패\n";
}

void ErrorState::onExit(RVCController& ctx) {
}

void ErrorState::handleObstacle(RVCController& ctx, bool front, bool left, bool right) {
    std::cout << "[ErrorState] handleObstacle을 무시합니다.\n";
}

void ErrorState::handleDust(RVCController& ctx, bool detected) {
    std::cout << "[ErrorState] handleDust를 무시합니다.\n";
}

void ErrorState::handlePowerOff(RVCController& ctx) {
    IRVCState* offState = nullptr; // 추후 변경 예정
    ctx.setState(offState);
}
} // namespace rvc