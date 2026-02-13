#pragma once
#include <glm/glm.hpp>

struct PointerState {
  bool      down     = false;
  bool      pressed  = false;
  bool      released = false;
  glm::vec2 worldPos{0.0f};
};
