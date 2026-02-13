#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <memory>
#include <string>

class PhysicsSystem {
public:
  virtual ~PhysicsSystem() = default;

  virtual void init(entt::registry&) {}

  virtual void fixedUpdate(entt::registry& reg, float fixedDt) = 0;

  virtual const char* name() const = 0;

  bool enabled = true;
};

class PhysicsWorld {
public:
  PhysicsWorld() = default;

  template<typename T, typename... Args>
  T& addSystem(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    T& ref   = *ptr;
    m_systems.push_back(std::move(ptr));
    return ref;
  }

  void init(entt::registry& reg);
  void update(entt::registry& reg, float dt);

  void  setFixedTimestep(float dt) { m_fixedTimestep = dt; }
  float getFixedTimestep() const   { return m_fixedTimestep; }

private:
  std::vector<std::unique_ptr<PhysicsSystem>> m_systems;
  float m_fixedTimestep = 1.0f / 60.0f;
  float m_accumulator   = 0.0f;
};
