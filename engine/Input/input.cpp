#include "input.hpp"
#include "entt/entt.hpp"
#include "GLFW/glfw3.h"
#include "renderer/window.hpp"

void InputSystem::processInput(entt::registry& reg) {
  if(glfwGetKey(m_window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(m_window.getHandle(), true);
  }
}
