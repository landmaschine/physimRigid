#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

struct RigidBody2D {
  glm::vec2 velocity{0.0f};
  glm::vec2 acceleration{0.0f};
  glm::vec2 force{0.0f};       

  float mass         = 1.0f;
  float invMass      = 1.0f;
  float restitution  = 0.2f;     
  float friction       = 0.3f;
  float linearDamping   = 0.01f;
  float angularDamping  = 0.05f;
  float maxLinearSpeed  = 50.0f;  

  float angularVelocity = 0.0f;
  float torque          = 0.0f;
  float inertia         = 1.0f;
  float invInertia      = 1.0f;

  bool  isStatic = false;

  void setMass(float m) {
    mass    = m;
    invMass = (m > 0.0f && !isStatic) ? 1.0f / m : 0.0f;
  }

  void setStatic(bool s) {
    isStatic = s;
    if (s) {
      invMass         = 0.0f;
      invInertia      = 0.0f;
      velocity        = {0, 0};
      angularVelocity = 0.0f;
    } else {
      invMass    = mass    > 0.0f ? 1.0f / mass    : 0.0f;
      invInertia = inertia > 0.0f ? 1.0f / inertia : 0.0f;
    }
  }

  void addForce(const glm::vec2& f) { force += f; }
  void clearForces() { force = {0, 0}; torque = 0.0f; }
};

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
