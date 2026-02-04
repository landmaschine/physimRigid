#pragma once

#include "entt/entity/fwd.hpp"
class Window;

class InputSystem {
public:
  InputSystem(Window& window) : m_window(window) {}
  ~InputSystem() = default;
  
  void processInput(entt::registry& reg);

private:
  Window& m_window;

};