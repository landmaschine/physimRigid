#pragma once
#include "../physicsSystem.hpp"
#include "components/physics_components.hpp"
#include <glm/glm.hpp>

class GravitySystem : public PhysicsSystem {
public:
  explicit GravitySystem(glm::vec2 gravity = {0.0f, -9.81f})
    : m_gravity(gravity) {}

  void fixedUpdate(entt::registry& reg, float /*fixedDt*/) override {
    auto view = reg.view<RigidBody2D>();
    for (auto [entity, rb] : view.each()) {
      if (!rb.isStatic)
        rb.addForce(m_gravity * rb.mass);
    }
  }

  const char* name() const override { return "Gravity"; }

  glm::vec2 m_gravity;
};
