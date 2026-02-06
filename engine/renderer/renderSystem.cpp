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
    ERRLOG("Failed to load basic shaders");
    std::exit(-1);
  }

  m_gridShader = std::make_unique<Shader>();
  if (!m_gridShader->load("shaders/grid.vert", "shaders/grid.frag")) {
    ERRLOG("Failed to load grid shaders");
    std::exit(-1);
  }

  glGenVertexArrays(1, &m_gridVAO);
  glGenBuffers(1, &m_gridVBO);
  glBindVertexArray(m_gridVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
  
  glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

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

void RendererSystem::renderGrid() {
  float aspect = static_cast<float>(m_window.width()) / static_cast<float>(m_window.height());
  float worldHeight = 10.0f;
  float worldWidth  = worldHeight * aspect;
  float hw = worldWidth  / 2.0f;
  float hh = worldHeight / 2.0f;

  glm::mat4 projection = glm::ortho(-hw, hw, -hh, hh, -1.0f, 1.0f);

  float quad[] = {
    -hw, -hh,
     hw, -hh,
     hw,  hh,
    -hw, -hh,
     hw,  hh,
    -hw,  hh
  };
  glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);

  m_gridShader->use();
  m_gridShader->setMat4("u_projection", projection);
  m_gridShader->setFloat("u_gridSpacing", gridSpacing);
  m_gridShader->setFloat("u_lineWidth", gridLineWidth);
  m_gridShader->setVec3("u_gridColor", glm::vec3(0.35f, 0.35f, 0.35f));

  glBindVertexArray(m_gridVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

void RendererSystem::render(entt::registry& reg) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderGrid();

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
    m_shader->setVec3("u_color", glm::vec3(1.0f, 1.0f, 1.0f));

    glBindVertexArray(render.VAO);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINE_LOOP, 0, render.vertexCount);
  }

  glBindVertexArray(0);
}

void RendererSystem::shutdown() {
  m_shader.reset();
  m_gridShader.reset();
  if (m_gridVAO) { glDeleteVertexArrays(1, &m_gridVAO); m_gridVAO = 0; }
  if (m_gridVBO) { glDeleteBuffers(1, &m_gridVBO); m_gridVBO = 0; }
}