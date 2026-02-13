#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <algorithm>

struct ContactFeature {
  enum Type : uint8_t { VERTEX = 0, FACE = 1 };

  uint8_t indexA = 0; 
  Type    typeA  = VERTEX;
  uint8_t indexB = 0;
  Type    typeB  = VERTEX;

  uint32_t key() const {
    return (uint32_t(typeA) << 24) | (uint32_t(indexA) << 16) |
           (uint32_t(typeB) << 8)  |  uint32_t(indexB);
  }
  bool operator==(const ContactFeature& o) const { return key() == o.key(); }
};

struct ContactPoint {
  glm::vec2 position{0.f};
  glm::vec2 localA{0.f}; 
  glm::vec2 localB{0.f};   
  float     penetration = 0.f; 
  ContactFeature feature;

  float normalImpulse  = 0.f;
  float tangentImpulse = 0.f;

  glm::vec2 rA{0.f};
  glm::vec2 rB{0.f}; 
  float     normalMass  = 0.f; 
  float     tangentMass = 0.f;
  float     velocityBias = 0.f;
};

struct ContactConstraint {
  entt::entity bodyA = entt::null;
  entt::entity bodyB = entt::null;
  glm::vec2    normal{0.f};
  ContactPoint points[2];
  int          pointCount = 0;
  float        friction    = 0.f;
  float        restitution = 0.f;
};

inline uint64_t contactPairKey(entt::entity a, entt::entity b) {
  auto lo = static_cast<uint32_t>(a);
  auto hi = static_cast<uint32_t>(b);
  if (lo > hi) std::swap(lo, hi);
  return (static_cast<uint64_t>(hi) << 32) | lo;
}

class ContactManager {
public:
  void update(const std::vector<ContactConstraint>& newContacts) {
    std::unordered_map<uint64_t, ContactConstraint> newMap;
    newMap.reserve(newContacts.size());

    for (auto nc : newContacts) { 
      uint64_t key = contactPairKey(nc.bodyA, nc.bodyB);
      auto it = m_contacts.find(key);
      if (it != m_contacts.end()) {
        warmMatch(nc, it->second);
      }
      newMap.emplace(key, nc);
    }
    m_contacts = std::move(newMap);
  }

  auto begin()       { return m_contacts.begin(); }
  auto end()         { return m_contacts.end();   }
  auto begin() const { return m_contacts.begin(); }
  auto end()   const { return m_contacts.end();   }
  size_t size() const { return m_contacts.size(); }
  bool empty()  const { return m_contacts.empty();}

  void clear() { m_contacts.clear(); }

private:
  void warmMatch(ContactConstraint& nc, const ContactConstraint& oc) {
    for (int i = 0; i < nc.pointCount; ++i) {
      for (int j = 0; j < oc.pointCount; ++j) {
        if (nc.points[i].feature.key() == oc.points[j].feature.key()) {
          nc.points[i].normalImpulse  = oc.points[j].normalImpulse;
          nc.points[i].tangentImpulse = oc.points[j].tangentImpulse;
          break;
        }
      }
    }
  }

  std::unordered_map<uint64_t, ContactConstraint> m_contacts;
};
