#pragma once
#include "../physicsSystem.hpp"
#include "../contact.hpp"
#include "../broadphase.hpp"
#include "../narrowphase.hpp"
#include "../collisionEvents.hpp"
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <vector>


class CollisionDetectionSystem : public PhysicsSystem {
public:
  void init(entt::registry& reg) override {
    if (!reg.ctx().contains<ContactManager>())
      reg.ctx().emplace<ContactManager>();
    if (!reg.ctx().contains<CollisionEvents>())
      reg.ctx().emplace<CollisionEvents>();
    if (!reg.ctx().contains<CollisionPairTracker>())
      reg.ctx().emplace<CollisionPairTracker>();
  }

  void fixedUpdate(entt::registry& reg, float /*fixedDt*/) override {
    auto& cm = reg.ctx().get<ContactManager>();

    m_bodies.clear();
    m_bpEntries.clear();

    {
      auto view = reg.view<TransformComponent, RigidBody2D>();
      for (auto [e, xf, rb] : view.each()) {
        CircleCollider* cc = reg.try_get<CircleCollider>(e);
        BoxCollider*    bc = reg.try_get<BoxCollider>(e);
        ConvexCollider* cv = reg.try_get<ConvexCollider>(e);
        if (!cc && !bc && !cv) continue;

        m_bodies.push_back({ e, &xf, &rb, cc, bc, cv });

        AABB aabb;
        if (cc) aabb = computeCircleAABB(xf, *cc);
        else if (bc) aabb = computeBoxAABB(xf, *bc);
        else if (cv) aabb = computeConvexAABB(xf, *cv);

        m_bpEntries.push_back({ e, aabb.fattened(0.01f) });
      }
    }

    sortAndSweep(m_bpEntries, m_pairs);

    m_bodyIndex.clear();
    m_bodyIndex.reserve(m_bodies.size());
    for (size_t i = 0; i < m_bodies.size(); ++i)
      m_bodyIndex[static_cast<uint32_t>(m_bodies[i].ent)] = i;

    m_newContacts.clear();
    m_newContacts.reserve(m_pairs.size());
    m_collisionEvents.clear();

    for (auto& [entA, entB] : m_pairs) {
      auto itA = m_bodyIndex.find(static_cast<uint32_t>(entA));
      auto itB = m_bodyIndex.find(static_cast<uint32_t>(entB));
      if (itA == m_bodyIndex.end() || itB == m_bodyIndex.end()) continue;

      auto& A = m_bodies[itA->second];
      auto& B = m_bodies[itB->second];

      if (!isDynamic(*A.rb) && !isDynamic(*B.rb)) continue;

      if (!shouldCollide(A.rb->filter, B.rb->filter)) continue;

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
        m_newContacts.push_back(std::move(*contact));

        auto& cc = m_newContacts.back();
        CollisionEvent ev;
        ev.entityA      = cc.bodyA;
        ev.entityB      = cc.bodyB;
        ev.normal       = cc.normal;
        ev.penetration  = cc.points[0].penetration;
        ev.contactPoint = cc.points[0].position;
        m_collisionEvents.push_back(ev);
      }
    }

    cm.update(m_newContacts);

    if (reg.ctx().contains<CollisionPairTracker>()) {
      auto& tracker = reg.ctx().get<CollisionPairTracker>();
      auto& events  = reg.ctx().get<CollisionEvents>();
      tracker.update(m_collisionEvents, events);
    }
  }

  const char* name() const override { return "CollisionDetection"; }

private:
  struct Collidable {
    entt::entity      ent;
    TransformComponent* xf;
    RigidBody2D*       rb;
    CircleCollider*    circle  = nullptr;
    BoxCollider*       box     = nullptr;
    ConvexCollider*    convex  = nullptr;
  };

  std::vector<Collidable>                  m_bodies;
  std::vector<BroadphaseEntry>             m_bpEntries;
  std::vector<BroadphasePair>              m_pairs;
  std::unordered_map<uint32_t, size_t>     m_bodyIndex;
  std::vector<ContactConstraint>           m_newContacts;
  std::vector<CollisionEvent>              m_collisionEvents;
};
