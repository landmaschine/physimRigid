#pragma once
#include "components/physics_components.hpp"
#include <entt/entt.hpp>
#include <cmath>

inline void computeBodyInertia(entt::registry& reg, entt::entity e) {
  if (!reg.all_of<RigidBody2D>(e)) return;
  auto& rb = reg.get<RigidBody2D>(e);
  if (rb.isStatic) {
    rb.inertia    = 0.0f;
    rb.invInertia = 0.0f;
    return;
  }

  float I = rb.mass * 0.1f; 

  if (reg.all_of<CircleCollider>(e)) {
    float r = reg.get<CircleCollider>(e).radius;
    I = 0.5f * rb.mass * r * r;
  }
  else if (reg.all_of<BoxCollider>(e)) {
    auto& b = reg.get<BoxCollider>(e);
    float w = b.halfExtents.x * 2.0f;
    float h = b.halfExtents.y * 2.0f;
    I = (1.0f / 12.0f) * rb.mass * (w * w + h * h);  
  }
  else if (reg.all_of<ConvexCollider>(e)) {
    auto& cv = reg.get<ConvexCollider>(e);
    int n = static_cast<int>(cv.vertices.size());
    if (n >= 3) {
      float totalArea = 0.0f;
      glm::vec2 centroid{0.0f};
      for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        float cross = cv.vertices[i].x * cv.vertices[j].y
                    - cv.vertices[j].x * cv.vertices[i].y;
        totalArea += cross;
        centroid  += (cv.vertices[i] + cv.vertices[j]) * cross;
      }
      totalArea *= 0.5f;
      if (std::abs(totalArea) > 1e-8f) {
        centroid /= (6.0f * totalArea);
        float sum = 0.0f;
        for (int i = 0; i < n; ++i) {
          int j = (i + 1) % n;
          glm::vec2 a = cv.vertices[i] - centroid;
          glm::vec2 b = cv.vertices[j] - centroid;
          float cross = std::abs(a.x * b.y - b.x * a.y);
          sum += cross * (glm::dot(a, a) + glm::dot(a, b) + glm::dot(b, b));
        }
        I = (rb.mass / 6.0f) * sum / std::abs(2.0f * totalArea);
      }
    }
  }

  rb.inertia    = I;
  rb.invInertia = I > 0.0f ? 1.0f / I : 0.0f;
}
