#pragma once
#include <cstdint>
#include <string>

struct GLFWwindow;

class Window{
public:
  Window(uint32_t width, uint32_t height, std::string title) {
    init(width, height, title);
  }
  ~Window() { shutdown(); }

  bool shouldClose();
  void pollEvents();

  GLFWwindow* getHandle() {
    return m_glfwWindow;
  }

  void* getNativeHandle() const;

  void* getNativeDisplayHandle() const;

  uint32_t width() const { return m_width; }
  uint32_t height() const { return m_height; }

  void setSize(uint32_t w, uint32_t h) { m_width = w; m_height = h; }

private:
  void init(uint32_t width, uint32_t height, std::string title);
  void shutdown();

private:
  uint32_t m_height;
  uint32_t m_width;
  GLFWwindow* m_glfwWindow = nullptr;
  std::string m_title;
};
