#include "renderSystem.hpp"

#include "entt/entt.hpp"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "logger/logger.hpp"
#include "renderer/window.hpp"
#include "renderer/shader.hpp"
#include "components/components.hpp"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

RendererSystem::RendererSystem(Window& window) : m_window(window) {
  init();
}

RendererSystem::~RendererSystem() {
  shutdown();
}

void RendererSystem::init() {
  if (!gladLoadGL(glfwGetProcAddress)) {
    ERRLOG("Failed to init GLAD");
    std::exit(-1);
  }

  glfwSetFramebufferSizeCallback(m_window.getHandle(), framebuffer_size_callback);

  m_shader = std::make_unique<Shader>();
  if (!m_shader->load("shaders/basic.vert", "shaders/basic.frag")) {
    ERRLOG("Failed to load shaders");
    std::exit(-1);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RendererSystem::initEntityBuffers(entt::registry& reg) {
  auto view = reg.view<RigidBodyComponent, RenderComponent>();
  for (auto entity : view) {
    auto& render = view.get<RenderComponent>(entity);
    if (render.initialized) continue;

    auto& body = view.get<RigidBodyComponent>(entity);
    if (!body.shape) continue;

    auto vertices = body.shape->getVertices();

    glGenVertexArrays(1, &render.VAO);
    glGenBuffers(1, &render.VBO);

    glBindVertexArray(render.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, render.VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    render.vertexCount = static_cast<int>(body.shape->getVertexCount());
    render.initialized = true;
  }
}

void RendererSystem::render(entt::registry& reg) {
  glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  initEntityBuffers(reg);

  m_shader->use();

  float aspect = static_cast<float>(m_window.width()) / static_cast<float>(m_window.height());
  float worldHeight = 10.0f;
  float worldWidth  = worldHeight * aspect;
  glm::mat4 projection = glm::ortho(-worldWidth  / 2.0f, worldWidth  / 2.0f,
                                     -worldHeight / 2.0f, worldHeight / 2.0f,
                                     -1.0f, 1.0f);
  m_shader->setMat4("u_projection", projection);

  auto view = reg.view<TransformComponent, RigidBodyComponent, RenderComponent>();
  for (auto entity : view) {
    auto& transform = view.get<TransformComponent>(entity);
    auto& render    = view.get<RenderComponent>(entity);
    if (!render.initialized || render.VAO == 0) continue;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(transform.pos, 0.0f));

    m_shader->setMat4("u_transform", model);
    m_shader->setVec3("u_color", render.color);

    glBindVertexArray(render.VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, render.vertexCount);
  }

  glBindVertexArray(0);
}

void RendererSystem::shutdown() {
  m_shader.reset();
}