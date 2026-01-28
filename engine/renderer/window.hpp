#pragma once

#include <cstdint>
#include <string>

#include "GLFW/glfw3.h"

class Window{
public:
  Window() = default;
  ~Window() {}

  void init(uint32_t width, uint32_t height, std::string title);
  void pollEvents();
  void shutdown();

private:
  uint32_t m_height;
  uint32_t m_width;
  GLFWwindow* m_glfwWindow;
  std::string m_title;

};
