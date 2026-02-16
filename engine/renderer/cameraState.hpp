#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include "components/transform.hpp"
#include "components/render_components.hpp"

struct CameraState {
  glm::vec2 position{0.0f};
  float     orthoSize = 5.0f;
  float     aspect    = 16.0f / 9.0f;

  float halfW() const { return orthoSize * aspect; }
  float halfH() const { return orthoSize; }
};

inline void updateCameraState(entt::registry& reg, float windowAspect) {
  if (!reg.ctx().contains<CameraState>())
    reg.ctx().emplace<CameraState>();

  auto& cam = reg.ctx().get<CameraState>();
  cam.aspect = windowAspect;

  auto view = reg.view<TransformComponent, CameraComponent>();
  for (auto e : view) {
    auto& cc = view.get<CameraComponent>(e);
    if (cc.primary) {
      cam.orthoSize = cc.orthoSize;
      cam.position  = view.get<TransformComponent>(e).position;
      return;
    }
  }
}
