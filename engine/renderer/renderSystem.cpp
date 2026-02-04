#include "renderSystem.hpp"

#include "entt/entt.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"

#include "logger/logger.hpp"
#include "renderer/window.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void RendererSystem::init() {
  if(!gladLoadGL(glfwGetProcAddress)) {
    ERRLOG("Failed to init GLAD");
    std::exit(-1);
  }

  glfwSetFramebufferSizeCallback(m_window.getHandle(), framebuffer_size_callback);

}

void RendererSystem::render(entt::registry& reg) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}