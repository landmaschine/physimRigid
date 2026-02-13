#pragma once
#include "../physicsSystem.hpp"
#include "../inertia.hpp"

class InertiaSystem : public PhysicsSystem {
public:
  void init(entt::registry& reg) override {
    auto view = reg.view<RigidBody2D>();
    for (auto entity : view) {
      computeBodyInertia(reg, entity);
    }
  }

  void fixedUpdate(entt::registry&, float) override {

  }

  const char* name() const override { return "Inertia"; }
};
