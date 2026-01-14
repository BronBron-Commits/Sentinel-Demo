#pragma once
#include <cstdint>
#include <memory>
#include "orchestra.h"

namespace demo {

struct DroneState : public sentinel::SystemState {
    double x;
    double y;
    double vx;
    double vy;
};

class DroneSystem : public sentinel::ISystem {
public:
    DroneSystem();

    void step(uint64_t tick) override;
    uint64_t hash() const override;

    std::unique_ptr<sentinel::SystemState> save_state() const override;
    void load_state(const sentinel::SystemState* state) override;

private:
    DroneState state;
};

} // namespace demo
