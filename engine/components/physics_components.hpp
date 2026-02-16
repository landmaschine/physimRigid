#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

enum class BodyType : uint8_t {
  Static    = 0,
  Kinematic = 1,
  Dynamic   = 2
};


struct CollisionFilter {
  uint16_t categoryBits = 0x0001; 
  uint16_t maskBits     = 0xFFFF; 
  int16_t  groupIndex   = 0; 
};

inline bool shouldCollide(const CollisionFilter& a, const CollisionFilter& b) {
  if (a.groupIndex != 0 && a.groupIndex == b.groupIndex)
    return a.groupIndex > 0;
  return (a.categoryBits & b.maskBits) != 0
      && (b.categoryBits & a.maskBits) != 0;
}

struct RigidBody2D {
  glm::vec2 velocity{0.0f};
  glm::vec2 force{0.0f};       

  float mass         = 1.0f;
  float invMass      = 1.0f;
  float restitution  = 0.2f;     
  float friction     = 0.3f;
  float linearDamping   = 0.01f;
  float angularDamping  = 0.05f;
  float maxLinearSpeed  = 50.0f;  

  float angularVelocity = 0.0f;
  float torque          = 0.0f;
  float inertia         = 1.0f;
  float invInertia      = 1.0f;

  BodyType type          = BodyType::Dynamic;
  bool     fixedRotation = false;

  CollisionFilter filter;
};

inline bool isStatic(const RigidBody2D& rb)    { return rb.type == BodyType::Static; }
inline bool isKinematic(const RigidBody2D& rb) { return rb.type == BodyType::Kinematic; }
inline bool isDynamic(const RigidBody2D& rb)   { return rb.type == BodyType::Dynamic; }

inline void syncBodyMass(RigidBody2D& rb) {
  if (rb.type != BodyType::Dynamic) {
    rb.invMass    = 0.0f;
    rb.invInertia = 0.0f;
  } else {
    rb.invMass    = rb.mass    > 0.0f ? 1.0f / rb.mass    : 0.0f;
    rb.invInertia = rb.fixedRotation ? 0.0f
                  : (rb.inertia > 0.0f ? 1.0f / rb.inertia : 0.0f);
  }
}

inline void setBodyMass(RigidBody2D& rb, float m) {
  rb.mass = m;
  syncBodyMass(rb);
}

inline void setBodyType(RigidBody2D& rb, BodyType t) {
  rb.type = t;
  if (t != BodyType::Dynamic) {
    rb.velocity        = {0, 0};
    rb.angularVelocity = 0.0f;
    rb.force           = {0, 0};
    rb.torque          = 0.0f;
  }
  syncBodyMass(rb);
}

inline void setBodyStatic(RigidBody2D& rb, bool s) {
  setBodyType(rb, s ? BodyType::Static : BodyType::Dynamic);
}

inline void addForce(RigidBody2D& rb, const glm::vec2& f) { rb.force += f; }

inline void addForceAtPoint(RigidBody2D& rb,
                            const glm::vec2& f,
                            const glm::vec2& worldPoint,
                            const glm::vec2& bodyPosition) {
  rb.force  += f;
  glm::vec2 r = worldPoint - bodyPosition;
  rb.torque += r.x * f.y - r.y * f.x;
}

inline void addTorque(RigidBody2D& rb, float t) { rb.torque += t; }

inline void clearForces(RigidBody2D& rb) {
  rb.force  = {0, 0};
  rb.torque = 0.0f;
}

struct CircleCollider {
  float     radius = 0.5f;
  glm::vec2 offset{0.0f};  
};

struct BoxCollider {
  glm::vec2 halfExtents{0.5f};
  glm::vec2 offset{0.0f};       
};

struct ConvexCollider {
  std::vector<glm::vec2> vertices; 
  glm::vec2 offset{0.0f}; 

  void ensureCCW() {
    float area = 0.0f;
    for (size_t i = 0; i < vertices.size(); ++i) {
      size_t j = (i + 1) % vertices.size();
      area += vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y;
    }
    if (area < 0.0f) std::reverse(vertices.begin(), vertices.end());
  }
};
