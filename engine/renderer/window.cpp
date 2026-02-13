#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "logger/logger.hpp"

#ifdef _WIN32
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#  ifdef ERROR
#    undef ERROR
#  endif
#endif

void Window::init(uint32_t width, uint32_t height, std::string title) {
  m_width = width;
  m_height = height;
  m_title = title;

  if(glfwInit() == 0) {
    ERRLOG("Failed to init glfw!");
    std::exit(-1);
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_glfwWindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
  if(m_glfwWindow == nullptr) {
    ERRLOG("Failed to create glfw Window handle!");
    glfwTerminate();
    std::exit(1);
  }

  glfwSetWindowUserPointer(m_glfwWindow, this);
}

void Window::pollEvents() {
  glfwPollEvents();
}  

bool Window::shouldClose() {
  return glfwWindowShouldClose(m_glfwWindow);
}

void* Window::getNativeHandle() const {
#ifdef _WIN32
  return glfwGetWin32Window(m_glfwWindow);
#else
  return nullptr;
#endif
}

void* Window::getNativeDisplayHandle() const {
  return nullptr; 
}

void Window::shutdown() {
  if (m_glfwWindow) {
    glfwDestroyWindow(m_glfwWindow);
    m_glfwWindow = nullptr;
  }
  glfwTerminate();
}