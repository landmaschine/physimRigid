#pragma once
#include "../physicsSystem.hpp"
#include "../contact.hpp"
#include "mouseGrab.hpp"
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <algorithm>

class ConstraintSolverSystem : public PhysicsSystem {
public:
  int   velocityIterations = 10;
  int   positionIterations = 4;
  float baumgarte          = 0.2f;  
  float slop               = 0.005f;  
  float maxPositionCorrection = 0.2f; 
  float restitutionThreshold  = 1.0f; 

  void fixedUpdate(entt::registry& reg, float dt) override {
    if (!reg.ctx().contains<ContactManager>()) return;
    auto& cm = reg.ctx().get<ContactManager>();

    MouseGrabState* grab = nullptr;
    if (reg.ctx().contains<MouseGrabState>()) {
      auto& ms = reg.ctx().get<MouseGrabState>();
      if (ms.active && reg.valid(ms.grabbed))
        grab = &ms;
    }

    integrateVelocities(reg, dt);

    if (cm.empty() && !grab) {
      integratePositions(reg, dt);
      clearForces(reg);
      return;
    }

    for (auto& [key, cc] : cm)
      preStep(reg, cc, dt);

    for (auto& [key, cc] : cm)
      warmStart(reg, cc);

    for (int i = 0; i < velocityIterations; ++i) {
      if (grab)
        MouseGrabSystem::solveGrabStep(reg, *grab);

      for (auto& [key, cc] : cm)
        solveVelocity(reg, cc);
    }

    integratePositions(reg, dt);

    for (int i = 0; i < positionIterations; ++i) {
      for (auto& [key, cc] : cm)
        solvePosition(reg, cc);
    }

    clearForces(reg);
  }

  const char* name() const override { return "ConstraintSolver"; }

private:
  static float cross2(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
  }
  static glm::vec2 cross2(float s, const glm::vec2& v) {
    return { -s * v.y, s * v.x };
  }

  void integrateVelocities(entt::registry& reg, float dt) {
    auto view = reg.view<RigidBody2D>();
    for (auto [entity, rb] : view.each()) {
      if (rb.isStatic) continue;

      rb.velocity += (rb.force * rb.invMass) * dt;

      rb.angularVelocity += (rb.torque * rb.invInertia) * dt;

      rb.velocity        *= 1.f / (1.f + rb.linearDamping  * dt);
      rb.angularVelocity *= 1.f / (1.f + rb.angularDamping * dt);

      float speed2 = glm::dot(rb.velocity, rb.velocity);
      if (speed2 > rb.maxLinearSpeed * rb.maxLinearSpeed)
        rb.velocity *= rb.maxLinearSpeed / std::sqrt(speed2);
    }
  }

  void integratePositions(entt::registry& reg, float dt) {
    auto view = reg.view<RigidBody2D, TransformComponent>();
    for (auto [entity, rb, xf] : view.each()) {
      if (rb.isStatic) continue;
      xf.position += rb.velocity * dt;
      xf.rotation += rb.angularVelocity * dt;
    }
  }

  void clearForces(entt::registry& reg) {
    auto view = reg.view<RigidBody2D>();
    for (auto [entity, rb] : view.each())
      rb.clearForces();
  }

  void preStep(entt::registry& reg, ContactConstraint& cc, float dt) {
    auto& xfA = reg.get<TransformComponent>(cc.bodyA);
    auto& rbA = reg.get<RigidBody2D>(cc.bodyA);
    auto& xfB = reg.get<TransformComponent>(cc.bodyB);
    auto& rbB = reg.get<RigidBody2D>(cc.bodyB);

    for (int i = 0; i < cc.pointCount; ++i) {
      auto& pt = cc.points[i];

      pt.rA = pt.position - xfA.position;
      pt.rB = pt.position - xfB.position;

      float rnA = cross2(pt.rA, cc.normal);
      float rnB = cross2(pt.rB, cc.normal);
      float kn = rbA.invMass + rbB.invMass
               + rbA.invInertia * rnA * rnA
               + rbB.invInertia * rnB * rnB;
      pt.normalMass = (kn > 0.f) ? 1.f / kn : 0.f;

      glm::vec2 tangent = { -cc.normal.y, cc.normal.x };
      float rtA = cross2(pt.rA, tangent);
      float rtB = cross2(pt.rB, tangent);
      float kt = rbA.invMass + rbB.invMass
               + rbA.invInertia * rtA * rtA
               + rbB.invInertia * rtB * rtB;
      pt.tangentMass = (kt > 0.f) ? 1.f / kt : 0.f;

      glm::vec2 vA = rbA.velocity + cross2(rbA.angularVelocity, pt.rA);
      glm::vec2 vB = rbB.velocity + cross2(rbB.angularVelocity, pt.rB);
      float vRel = glm::dot(vB - vA, cc.normal);

      pt.velocityBias = 0.f;
      if (vRel < -restitutionThreshold)
        pt.velocityBias = -cc.restitution * vRel;
    }
  }

  void warmStart(entt::registry& reg, ContactConstraint& cc) {
    auto& rbA = reg.get<RigidBody2D>(cc.bodyA);
    auto& rbB = reg.get<RigidBody2D>(cc.bodyB);

    glm::vec2 tangent = { -cc.normal.y, cc.normal.x };

    for (int i = 0; i < cc.pointCount; ++i) {
      auto& pt = cc.points[i];
      glm::vec2 P = pt.normalImpulse * cc.normal
                   + pt.tangentImpulse * tangent;

      rbA.velocity        -= rbA.invMass    * P;
      rbA.angularVelocity -= rbA.invInertia * cross2(pt.rA, P);
      rbB.velocity        += rbB.invMass    * P;
      rbB.angularVelocity += rbB.invInertia * cross2(pt.rB, P);
    }
  }

  void solveVelocity(entt::registry& reg, ContactConstraint& cc) {
    auto& xfA = reg.get<TransformComponent>(cc.bodyA);
    auto& rbA = reg.get<RigidBody2D>(cc.bodyA);
    auto& xfB = reg.get<TransformComponent>(cc.bodyB);
    auto& rbB = reg.get<RigidBody2D>(cc.bodyB);

    glm::vec2 tangent = { -cc.normal.y, cc.normal.x };

    for (int i = 0; i < cc.pointCount; ++i) {
      auto& pt = cc.points[i];

      glm::vec2 vA = rbA.velocity + cross2(rbA.angularVelocity, pt.rA);
      glm::vec2 vB = rbB.velocity + cross2(rbB.angularVelocity, pt.rB);
      float vt = glm::dot(vB - vA, tangent);

      float lambda = pt.tangentMass * (-vt);

      float maxFriction = cc.friction * pt.normalImpulse;
      float oldAccum = pt.tangentImpulse;
      pt.tangentImpulse = glm::clamp(oldAccum + lambda,
                                      -maxFriction, maxFriction);
      lambda = pt.tangentImpulse - oldAccum;

      glm::vec2 P = lambda * tangent;
      rbA.velocity        -= rbA.invMass    * P;
      rbA.angularVelocity -= rbA.invInertia * cross2(pt.rA, P);
      rbB.velocity        += rbB.invMass    * P;
      rbB.angularVelocity += rbB.invInertia * cross2(pt.rB, P);
    }

    for (int i = 0; i < cc.pointCount; ++i) {
      auto& pt = cc.points[i];

      glm::vec2 vA = rbA.velocity + cross2(rbA.angularVelocity, pt.rA);
      glm::vec2 vB = rbB.velocity + cross2(rbB.angularVelocity, pt.rB);
      float vn = glm::dot(vB - vA, cc.normal);

      float lambda = pt.normalMass * (-vn + pt.velocityBias);

      float oldAccum = pt.normalImpulse;
      pt.normalImpulse = std::max(oldAccum + lambda, 0.f);
      lambda = pt.normalImpulse - oldAccum;

      glm::vec2 P = lambda * cc.normal;
      rbA.velocity        -= rbA.invMass    * P;
      rbA.angularVelocity -= rbA.invInertia * cross2(pt.rA, P);
      rbB.velocity        += rbB.invMass    * P;
      rbB.angularVelocity += rbB.invInertia * cross2(pt.rB, P);
    }
  }

  void solvePosition(entt::registry& reg, ContactConstraint& cc) {
    auto& xfA = reg.get<TransformComponent>(cc.bodyA);
    auto& rbA = reg.get<RigidBody2D>(cc.bodyA);
    auto& xfB = reg.get<TransformComponent>(cc.bodyB);
    auto& rbB = reg.get<RigidBody2D>(cc.bodyB);

    for (int i = 0; i < cc.pointCount; ++i) {
      auto& pt = cc.points[i];

      float cosA = std::cos(xfA.rotation), sinA = std::sin(xfA.rotation);
      float cosB = std::cos(xfB.rotation), sinB = std::sin(xfB.rotation);

      glm::vec2 rA = { cosA * pt.localA.x - sinA * pt.localA.y,
                        sinA * pt.localA.x + cosA * pt.localA.y };
      glm::vec2 rB = { cosB * pt.localB.x - sinB * pt.localB.y,
                        sinB * pt.localB.x + cosB * pt.localB.y };

      glm::vec2 worldA = xfA.position + rA;
      glm::vec2 worldB = xfB.position + rB;

      float separation = glm::dot(worldB - worldA, cc.normal);

      float C = std::min(separation + slop, 0.f);
      if (C >= 0.f) continue;

      float rnA = cross2(rA, cc.normal);
      float rnB = cross2(rB, cc.normal);
      float K = rbA.invMass + rbB.invMass
              + rbA.invInertia * rnA * rnA
              + rbB.invInertia * rnB * rnB;
      if (K <= 0.f) continue;

      float correction = std::min(-baumgarte * C / K, maxPositionCorrection);

      glm::vec2 P = correction * cc.normal;
      xfA.position -= rbA.invMass * P;
      xfB.position += rbB.invMass * P;
      xfA.rotation -= rbA.invInertia * rnA * correction;
      xfB.rotation += rbB.invInertia * rnB * correction;
    }
  }
};
