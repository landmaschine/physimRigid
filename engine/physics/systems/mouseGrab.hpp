#pragma once
#include "../physicsSystem.hpp"
#include "../pointerState.hpp"
#include "components/transform.hpp"
#include "components/physics_components.hpp"
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <cmath>
#include <limits>

struct MouseGrabState {
  bool          active    = false;
  entt::entity  grabbed   = entt::null;
  glm::vec2     localAnchor{0.f};
  glm::vec2     target{0.f};

  glm::vec2     rArm{0.f};
  glm::mat2     massMatrix{0.f};
  glm::vec2     impulseAccum{0.f};
  glm::vec2     bias{0.f};
  float         gamma     = 0.f;
  float         maxImpulse = 0.f;
};

class MouseGrabSystem : public PhysicsSystem {
public:
  float frequency    = 5.f;
  float dampingRatio = 1.f;
  float maxForce     = 500.f;

  void init(entt::registry& reg) override {
    if (!reg.ctx().contains<PointerState>())
      reg.ctx().emplace<PointerState>();
    if (!reg.ctx().contains<MouseGrabState>())
      reg.ctx().emplace<MouseGrabState>();
  }

  void fixedUpdate(entt::registry& reg, float fixedDt) override {
    if (!reg.ctx().contains<PointerState>()) return;
    auto& ps = reg.ctx().get<PointerState>();
    auto& ms = reg.ctx().get<MouseGrabState>();

    if (ps.pressed && !ms.active)
      tryGrab(reg, ms, ps);

    if (ps.released && ms.active) {
      ms.active  = false;
      ms.grabbed = entt::null;
      ms.impulseAccum = { 0.f, 0.f };
    }

    if (ms.active && reg.valid(ms.grabbed)) {
      ms.target = ps.worldPos;
      preStepGrab(reg, ms, fixedDt);
    }
  }

  const char* name() const override { return "MouseGrab"; }

  static void solveGrabStep(entt::registry& reg, MouseGrabState& ms) {
    auto& rb = reg.get<RigidBody2D>(ms.grabbed);

    glm::vec2 vAnchor = rb.velocity + cross2(rb.angularVelocity, ms.rArm);
    glm::vec2 Cdot = vAnchor + ms.bias + ms.gamma * ms.impulseAccum;
    glm::vec2 impulse = -(ms.massMatrix * Cdot);

    glm::vec2 oldAccum = ms.impulseAccum;
    ms.impulseAccum += impulse;
    float mag = glm::length(ms.impulseAccum);
    if (mag > ms.maxImpulse)
      ms.impulseAccum *= ms.maxImpulse / mag;
    impulse = ms.impulseAccum - oldAccum;

    rb.velocity        += rb.invMass    * impulse;
    rb.angularVelocity += rb.invInertia * cross2(ms.rArm, impulse);
  }

private:
  static glm::vec2 cross2(float s, const glm::vec2& v) {
    return { -s * v.y, s * v.x };
  }
  static float cross2(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
  }

  void tryGrab(entt::registry& reg, MouseGrabState& ms, const PointerState& ps) {
    float bestDist2 = std::numeric_limits<float>::max();
    entt::entity bestEnt = entt::null;
    glm::vec2 bestLocal{ 0.f };

    auto view = reg.view<TransformComponent, RigidBody2D>();
    for (auto [e, xf, rb] : view.each()) {
      if (rb.isStatic) continue;

      glm::vec2 diff = ps.worldPos - xf.position;
      float d2 = glm::dot(diff, diff);
      bool inside = false;

      if (reg.all_of<CircleCollider>(e)) {
        auto& c = reg.get<CircleCollider>(e);
        float r = c.radius * std::max(xf.scale.x, xf.scale.y);
        inside = d2 <= r * r * 1.2f;
      }
      if (!inside && reg.all_of<BoxCollider>(e)) {
        auto& bc = reg.get<BoxCollider>(e);
        float cosR = std::cos(-xf.rotation), sinR = std::sin(-xf.rotation);
        glm::vec2 local = {
          cosR * diff.x - sinR * diff.y,
          sinR * diff.x + cosR * diff.y
        };
        glm::vec2 half = bc.halfExtents * xf.scale;
        inside = std::abs(local.x) <= half.x * 1.1f
              && std::abs(local.y) <= half.y * 1.1f;
      }
      if (!inside && reg.all_of<ConvexCollider>(e)) {
        auto& pc = reg.get<ConvexCollider>(e);
        if (pc.vertices.size() >= 3) {
          float cosR = std::cos(-xf.rotation), sinR = std::sin(-xf.rotation);
          glm::vec2 local = {
            cosR * diff.x - sinR * diff.y,
            sinR * diff.x + cosR * diff.y
          };
          bool allPos = true, allNeg = true;
          size_t n = pc.vertices.size();
          for (size_t i = 0; i < n; ++i) {
            glm::vec2 a = pc.vertices[i] * xf.scale;
            glm::vec2 b = pc.vertices[(i + 1) % n] * xf.scale;
            float cr = (b.x - a.x) * (local.y - a.y) - (b.y - a.y) * (local.x - a.x);
            if (cr < 0.f) allPos = false;
            if (cr > 0.f) allNeg = false;
          }
          inside = allPos || allNeg;
        }
      }

      if (inside && d2 < bestDist2) {
        bestDist2 = d2;
        bestEnt   = e;
        float cosR = std::cos(-xf.rotation), sinR = std::sin(-xf.rotation);
        bestLocal = {
          cosR * diff.x - sinR * diff.y,
          sinR * diff.x + cosR * diff.y
        };
      }
    }

    if (bestEnt != entt::null) {
      ms.active       = true;
      ms.grabbed      = bestEnt;
      ms.localAnchor  = bestLocal;
      ms.target       = ps.worldPos;
      ms.impulseAccum = { 0.f, 0.f };
    }
  }

  void preStepGrab(entt::registry& reg, MouseGrabState& ms, float dt) {
    auto& xf = reg.get<TransformComponent>(ms.grabbed);
    auto& rb = reg.get<RigidBody2D>(ms.grabbed);

    float cosR = std::cos(xf.rotation), sinR = std::sin(xf.rotation);
    ms.rArm = glm::vec2{
      cosR * ms.localAnchor.x - sinR * ms.localAnchor.y,
      sinR * ms.localAnchor.x + cosR * ms.localAnchor.y
    };

    float rx = ms.rArm.x, ry = ms.rArm.y;
    float invM = rb.invMass, invI = rb.invInertia;

    float omega     = 2.f * 3.14159265f * frequency;
    float c_damping = 2.f * rb.mass * dampingRatio * omega;
    float k_spring  = rb.mass * omega * omega;

    ms.gamma = 1.f / (dt * (c_damping + dt * k_spring));
    float beta = dt * k_spring * ms.gamma;

    glm::mat2 K;
    K[0][0] = invM + invI * ry * ry + ms.gamma;
    K[0][1] = -invI * rx * ry;
    K[1][0] = -invI * rx * ry;
    K[1][1] = invM + invI * rx * rx + ms.gamma;

    float det = K[0][0] * K[1][1] - K[0][1] * K[1][0];
    if (std::abs(det) > 1e-12f) {
      float invDet = 1.f / det;
      ms.massMatrix = glm::mat2(
         K[1][1] * invDet, -K[0][1] * invDet,
        -K[1][0] * invDet,  K[0][0] * invDet
      );
    } else {
      ms.massMatrix = glm::mat2(0.f);
    }

    glm::vec2 worldAnchor = xf.position + ms.rArm;
    glm::vec2 error = worldAnchor - ms.target;
    ms.bias = beta * error;
    ms.maxImpulse = maxForce * dt;

    rb.velocity        += rb.invMass    * ms.impulseAccum;
    rb.angularVelocity += rb.invInertia * cross2(ms.rArm, ms.impulseAccum);
  }
};
