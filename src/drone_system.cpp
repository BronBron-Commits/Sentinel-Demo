#include "demo/drone_system.h"

namespace demo {

static uint64_t hash_combine(uint64_t h, uint64_t v) {
    h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    return h;
}

DroneSystem::DroneSystem() {
    state.x = 0.0;
    state.y = 0.0;
    state.vx = 1.0;
    state.vy = 0.5;
}

void DroneSystem::step(uint64_t) {
    state.x += state.vx;
    state.y += state.vy;
}

uint64_t DroneSystem::hash() const {
    uint64_t h = 0;
    h = hash_combine(h, static_cast<uint64_t>(state.x * 1000));
    h = hash_combine(h, static_cast<uint64_t>(state.y * 1000));
    h = hash_combine(h, static_cast<uint64_t>(state.vx * 1000));
    h = hash_combine(h, static_cast<uint64_t>(state.vy * 1000));
    return h;
}

std::unique_ptr<sentinel::SystemState> DroneSystem::save_state() const {
    return std::make_unique<DroneState>(state);
}

void DroneSystem::load_state(const sentinel::SystemState* s) {
    state = *static_cast<const DroneState*>(s);
}

} // namespace demo
