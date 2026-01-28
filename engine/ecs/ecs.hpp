#pragma once
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include <cstdint>
#include <entt/entt.hpp>

class Entity;

class Scene {
friend class Entity;

public:
  Scene() = default;
  ~Scene() = default;
  
  Entity createEntity();
  void onUpdate(float dt);

  entt::registry& getRegistry() { return m_registry; }
  const entt::registry& getRegistry() const { return m_registry; }

private:
  entt::registry m_registry;
};

class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

  template<typename T, typename... Args>
  T& addComponent(Args&&... args) {
    return m_Scene->m_registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
  }

  template<typename T>
  T& getComponent() {
    return m_Scene->m_registry.get<T>(m_EntityHandle);
  }

  template<typename T>
  bool hasComponent() {
    return m_Scene->m_registry.all_of<T>(m_EntityHandle);
  }

  template<typename T>
  void removeComponent() {
    m_Scene->m_registry.remove<T>(m_EntityHandle);
  }

  operator bool() const { return m_EntityHandle != entt::null; }
  operator entt::entity() const { return m_EntityHandle; }
  operator uint32_t() const { return (uint32_t)m_EntityHandle; }

private:
  entt::entity m_EntityHandle{ entt::null };
  Scene* m_Scene{  nullptr };
};
