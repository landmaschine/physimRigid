#include "input.hpp"
#include "GLFW/glfw3.h"
#include "renderer/window.hpp"

InputSystem::InputSystem(Window& window) : m_window(window) {}

bool InputSystem::isKeyDown(int key) const {
  auto it = m_keyState.find(key);
  return it != m_keyState.end() && it->second;
}

bool InputSystem::isKeyPressed(int key) const {
  auto curr = m_keyState.find(key);
  auto prev = m_prevKeyState.find(key);
  bool now = curr != m_keyState.end() && curr->second;
  bool was = prev != m_prevKeyState.end() && prev->second;
  return now && !was;
}

bool InputSystem::isKeyReleased(int key) const {
  auto curr = m_keyState.find(key);
  auto prev = m_prevKeyState.find(key);
  bool now = curr != m_keyState.end() && curr->second;
  bool was = prev != m_prevKeyState.end() && prev->second;
  return !now && was;
}

void InputSystem::processInput(float /*dt*/) {
  GLFWwindow* win = m_window.getHandle();

  m_prevKeyState = m_keyState;

  static const int trackedKeys[] = {
    GLFW_KEY_ESCAPE, GLFW_KEY_SPACE,
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F,
    GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
  };

  for (int key : trackedKeys) {
    m_keyState[key] = glfwGetKey(win, key) == GLFW_PRESS;
  }

  if (isKeyPressed(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(win, true);
  }

  double mx, my;
  glfwGetCursorPos(win, &mx, &my);

  float w = static_cast<float>(m_window.width());
  float h = static_cast<float>(m_window.height());

  mouseScreen = { static_cast<float>(mx), static_cast<float>(my) };

  float ndcX = static_cast<float>(mx) / w *  2.0f - 1.0f;
  float ndcY = static_cast<float>(my) / h * -2.0f + 1.0f;
  mouseWorld = {
    ndcX * worldHalfW + cameraPos.x,
    ndcY * worldHalfH + cameraPos.y
  };

  bool down = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  mousePressed  = down && !m_prevDown;
  mouseReleased = !down && m_prevDown;
  mouseDown     = down;
  m_prevDown    = down;
}
