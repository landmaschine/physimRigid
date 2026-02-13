#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

class Window;

class InputSystem {
public:
  InputSystem(Window& window);
  ~InputSystem() = default;

  void processInput(float dt);

  bool isKeyDown(int key) const;
  bool isKeyPressed(int key) const; 
  bool isKeyReleased(int key) const;

  bool      mouseDown     = false;
  bool      mousePressed  = false;
  bool      mouseReleased = false;
  glm::vec2 mouseScreen{0.0f};
  glm::vec2 mouseWorld{0.0f};

  float worldHalfW = 5.0f;
  float worldHalfH = 5.0f;
  glm::vec2 cameraPos{0.0f};

private:
  Window& m_window;
  bool m_prevDown = false;

  // per-frame key state tracking
  std::unordered_map<int, bool> m_keyState;
  std::unordered_map<int, bool> m_prevKeyState;
};