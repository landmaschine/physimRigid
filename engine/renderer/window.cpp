#include "window.hpp"

#include "GLFW/glfw3.h"
#include "logger/logger.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void Window::init(uint32_t width, uint32_t height, std::string title) {
  m_width = width;
  m_height = height;
  m_title = title;

  if(glfwInit() == 0) {
    ERRLOG("Failed to init glfw!");
    std::exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_glfwWindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
  if(m_glfwWindow == nullptr) {
    ERRLOG("Failed to create glfw Window handle!");
    glfwTerminate();
    std::exit(1);
  }

  glfwMakeContextCurrent(m_glfwWindow);

  if(!gladLoadGL(glfwGetProcAddress)) {
    ERRLOG("Failed to init GLAD");
    std::exit(-1);
  }

  glViewport(0, 0, m_width, m_height);

  glfwSetFramebufferSizeCallback(m_glfwWindow, framebuffer_size_callback);
}

void Window::pollEvents() {
  glfwPollEvents();
}  

void Window::swapBuffers() {
  glfwSwapBuffers(m_glfwWindow);
}

bool Window::shouldClose() {
  return glfwWindowShouldClose(m_glfwWindow);
}

void Window::shutdown() {
  glfwDestroyWindow(m_glfwWindow);
  glfwTerminate();
}