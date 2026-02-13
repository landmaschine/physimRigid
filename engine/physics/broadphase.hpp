#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <utility>

struct AABB {
  glm::vec2 min{0.f};
  glm::vec2 max{0.f};

  bool overlaps(const AABB& o) const {
    return min.x <= o.max.x && max.x >= o.min.x &&
           min.y <= o.max.y && max.y >= o.min.y;
  }

  AABB fattened(float margin) const {
    return { min - glm::vec2(margin), max + glm::vec2(margin) };
  }
};

inline AABB computeCircleAABB(const TransformComponent& xf,
                               const CircleCollider& cc) {
  float c = std::cos(xf.rotation), s = std::sin(xf.rotation);
  glm::vec2 worldOff = { c * cc.offset.x - s * cc.offset.y,
                          s * cc.offset.x + c * cc.offset.y };
  glm::vec2 center = xf.position + worldOff;
  float r = cc.radius * std::max(xf.scale.x, xf.scale.y);
  return { center - glm::vec2(r), center + glm::vec2(r) };
}

inline AABB computeBoxAABB(const TransformComponent& xf,
                            const BoxCollider& bc) {
  float c = std::cos(xf.rotation), s = std::sin(xf.rotation);
  glm::vec2 center = xf.position + glm::vec2{
    c * bc.offset.x - s * bc.offset.y,
    s * bc.offset.x + c * bc.offset.y };
  glm::vec2 half = bc.halfExtents * xf.scale;

  float ex = std::abs(c * half.x) + std::abs(s * half.y);
  float ey = std::abs(s * half.x) + std::abs(c * half.y);
  return { center - glm::vec2{ex, ey}, center + glm::vec2{ex, ey} };
}

inline AABB computeConvexAABB(const TransformComponent& xf,
                               const ConvexCollider& cv) {
  if (cv.vertices.empty())
    return { xf.position, xf.position };

  float co = std::cos(xf.rotation), si = std::sin(xf.rotation);
  glm::vec2 center = xf.position + glm::vec2{
    co * cv.offset.x - si * cv.offset.y,
    si * cv.offset.x + co * cv.offset.y };

  glm::vec2 mn{ 1e18f}, mx{-1e18f};
  for (auto& v : cv.vertices) {
    glm::vec2 sv = v * xf.scale;
    glm::vec2 wv = center + glm::vec2{co*sv.x - si*sv.y, si*sv.x + co*sv.y};
    mn = glm::min(mn, wv);
    mx = glm::max(mx, wv);
  }
  return { mn, mx };
}

struct BroadphaseEntry {
  entt::entity entity;
  AABB         aabb;
};

using BroadphasePair = std::pair<entt::entity, entt::entity>;

inline std::vector<BroadphasePair>
sortAndSweep(std::vector<BroadphaseEntry>& entries) {
  std::sort(entries.begin(), entries.end(),
    [](const BroadphaseEntry& a, const BroadphaseEntry& b) {
      return a.aabb.min.x < b.aabb.min.x;
    });

  std::vector<BroadphasePair> pairs;
  pairs.reserve(entries.size());  

  for (size_t i = 0; i < entries.size(); ++i) {
    for (size_t j = i + 1; j < entries.size(); ++j) {
      if (entries[j].aabb.min.x > entries[i].aabb.max.x)
        break;

      // Check y-axis overlap
      if (entries[i].aabb.min.y <= entries[j].aabb.max.y &&
          entries[i].aabb.max.y >= entries[j].aabb.min.y) {
        pairs.emplace_back(entries[i].entity, entries[j].entity);
      }
    }
  }

  return pairs;
}
