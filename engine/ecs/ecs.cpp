#include "ecs.hpp"

Entity Scene::createEntity() {
  Entity entity = {m_registry.create(), this};
  return entity;
}
