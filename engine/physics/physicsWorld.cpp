#include "physicsSystem.hpp"

void PhysicsWorld::init(entt::registry& reg) {
  for (auto& sys : m_systems)
    sys->init(reg);
}

void PhysicsWorld::update(entt::registry& reg, float dt) {
  m_accumulator += dt;

  const float maxAccum = m_fixedTimestep * 4.0f;
  if (m_accumulator > maxAccum)
    m_accumulator = maxAccum;

  while (m_accumulator >= m_fixedTimestep) {
    for (auto& sys : m_systems) {
      if (sys->enabled)
        sys->fixedUpdate(reg, m_fixedTimestep);
    }
    m_accumulator -= m_fixedTimestep;
  }
}
