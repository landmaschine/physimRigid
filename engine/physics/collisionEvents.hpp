#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_set>
#include <cstdint>

struct CollisionEvent {
  entt::entity entityA = entt::null;
  entt::entity entityB = entt::null;
  glm::vec2    normal{0.f};            
  glm::vec2    contactPoint{0.f};      
  float        penetration = 0.f;
};

struct CollisionEvents {
  std::vector<CollisionEvent> beginContacts;   
  std::vector<CollisionEvent> endContacts;     
  std::vector<CollisionEvent> stayContacts;    

  void clear() {
    beginContacts.clear();
    endContacts.clear();
    stayContacts.clear();
  }
};

class CollisionPairTracker {
public:
  using PairKey = uint64_t;

  static PairKey makeKey(entt::entity a, entt::entity b) {
    auto lo = static_cast<uint32_t>(a);
    auto hi = static_cast<uint32_t>(b);
    if (lo > hi) std::swap(lo, hi);
    return (static_cast<uint64_t>(hi) << 32) | lo;
  }

  void update(const std::vector<CollisionEvent>& currentContacts,
              CollisionEvents& events) {
    events.clear();

    std::unordered_set<PairKey> currentKeys;
    currentKeys.reserve(currentContacts.size());

    for (const auto& c : currentContacts) {
      PairKey k = makeKey(c.entityA, c.entityB);
      currentKeys.insert(k);

      if (m_activePairs.count(k) == 0) {
        events.beginContacts.push_back(c);
      } else {
        events.stayContacts.push_back(c);
      }
    }

    for (auto it = m_activePairs.begin(); it != m_activePairs.end(); ) {
      if (currentKeys.count(it->first) == 0) {
        events.endContacts.push_back(it->second);
        it = m_activePairs.erase(it);
      } else {
        ++it;
      }
    }

    for (const auto& c : currentContacts) {
      PairKey k = makeKey(c.entityA, c.entityB);
      m_activePairs[k] = c;
    }
  }

  void clear() { m_activePairs.clear(); }

private:
  std::unordered_map<PairKey, CollisionEvent> m_activePairs;
};
