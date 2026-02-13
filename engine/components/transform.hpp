#pragma once
#include <glm/glm.hpp>
#include <string>

struct TagComponent {
  std::string name;
};

struct TransformComponent {
  glm::vec2 position{0.0f};
  glm::vec2 scale{1.0f};
  float     rotation = 0.0f; 
};
