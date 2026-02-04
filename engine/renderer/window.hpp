#pragma once
#include <cstdint>
#include <string>

struct GLFWwindow;

class Window{
public:
  Window() = default;
  ~Window() {}

  void init(uint32_t width, uint32_t height, std::string title);
  void swapBuffers();
  bool shouldClose();
  void pollEvents();
  void shutdown();

  GLFWwindow* getHandle() {
    return m_glfwWindow;
  }

  uint32_t width() { return m_width; }
  uint32_t height() { return m_height; }

private:
  uint32_t m_height;
  uint32_t m_width;
  GLFWwindow* m_glfwWindow;
  std::string m_title;
};
