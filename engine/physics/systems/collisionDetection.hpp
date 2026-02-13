#pragma once
#include "../physicsSystem.hpp"
#include "../contact.hpp"
#include "../broadphase.hpp"
#include "../narrowphase.hpp"
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <vector>

// ═══════════════════════════════════════════════════════════════
//  Collision Detection System
//
//  Pipeline:
//    1. Compute AABBs for all collidable bodies
//    2. Sort-and-sweep broadphase → candidate pairs
//    3. SAT + clipping narrowphase → ContactConstraints
//    4. Feed into ContactManager for persistent warm-starting
//
//  The ContactManager survives across frames and matches contact
//  points by ContactFeature for impulse warm-starting.
// ═══════════════════════════════════════════════════════════════

class CollisionDetectionSystem : public PhysicsSystem {
public:
  void init(entt::registry& reg) override {
    if (!reg.ctx().contains<ContactManager>())
      reg.ctx().emplace<ContactManager>();
  }

  void fixedUpdate(entt::registry& reg, float /*fixedDt*/) override {
    auto& cm = reg.ctx().get<ContactManager>();

    struct Collidable {
      entt::entity      ent;
      TransformComponent* xf;
      RigidBody2D*       rb;
      CircleCollider*    circle  = nullptr;
      BoxCollider*       box     = nullptr;
      ConvexCollider*    convex  = nullptr;
    };

    std::vector<Collidable> bodies;
    std::vector<BroadphaseEntry> bpEntries;

    {
      auto view = reg.view<TransformComponent, RigidBody2D>();
      for (auto [e, xf, rb] : view.each()) {
        CircleCollider* cc = reg.all_of<CircleCollider>(e)
                              ? &reg.get<CircleCollider>(e) : nullptr;
        BoxCollider*    bc = reg.all_of<BoxCollider>(e)
                              ? &reg.get<BoxCollider>(e) : nullptr;
        ConvexCollider* cv = reg.all_of<ConvexCollider>(e)
                              ? &reg.get<ConvexCollider>(e) : nullptr;
        if (!cc && !bc && !cv) continue;

        bodies.push_back({ e, &xf, &rb, cc, bc, cv });

        AABB aabb;
        if (cc) aabb = computeCircleAABB(xf, *cc);
        else if (bc) aabb = computeBoxAABB(xf, *bc);
        else if (cv) aabb = computeConvexAABB(xf, *cv);

        bpEntries.push_back({ e, aabb.fattened(0.01f) });
      }
    }

    auto pairs = sortAndSweep(bpEntries);

    std::unordered_map<uint32_t, size_t> bodyIndex;
    bodyIndex.reserve(bodies.size());
    for (size_t i = 0; i < bodies.size(); ++i)
      bodyIndex[static_cast<uint32_t>(bodies[i].ent)] = i;

    std::vector<ContactConstraint> newContacts;
    newContacts.reserve(pairs.size());

    for (auto& [entA, entB] : pairs) {
      auto itA = bodyIndex.find(static_cast<uint32_t>(entA));
      auto itB = bodyIndex.find(static_cast<uint32_t>(entB));
      if (itA == bodyIndex.end() || itB == bodyIndex.end()) continue;

      auto& A = bodies[itA->second];
      auto& B = bodies[itB->second];

      if (A.rb->isStatic && B.rb->isStatic) continue;

      bool aCircle = A.circle != nullptr;
      bool bCircle = B.circle != nullptr;
      bool aPoly   = A.box || A.convex;
      bool bPoly   = B.box || B.convex;

      std::optional<ContactConstraint> contact;

      if (aCircle && bCircle) {
        contact = narrowphase::circleVsCircle(
          A.ent, *A.xf, *A.circle, B.ent, *B.xf, *B.circle);
      } else if (aCircle && bPoly) {
        contact = narrowphase::circleVsPoly(
          A.ent, *A.xf, *A.circle, B.ent, *B.xf, B.box, B.convex, false);
      } else if (aPoly && bCircle) {
        contact = narrowphase::circleVsPoly(
          B.ent, *B.xf, *B.circle, A.ent, *A.xf, A.box, A.convex, true);
      } else if (aPoly && bPoly) {
        contact = narrowphase::polyVsPoly(
          A.ent, *A.xf, A.box, A.convex,
          B.ent, *B.xf, B.box, B.convex);
      }

      if (contact) {
        contact->friction    = std::sqrt(A.rb->friction * B.rb->friction);
        contact->restitution = std::max(A.rb->restitution, B.rb->restitution);
        newContacts.push_back(std::move(*contact));
      }
    }

    cm.update(newContacts);
  }

  const char* name() const override { return "CollisionDetection"; }
};
