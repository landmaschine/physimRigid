#pragma once
#include <glm/glm.hpp>

struct SpriteComponent {
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec2 size{1.0f, 1.0f}; 
  int       sortOrder = 0;   
};

struct CameraComponent {
  float     orthoSize = 5.0f;  
  bool      primary   = true; 
};
