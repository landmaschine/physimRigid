#pragma once

#include "entt/entity/fwd.hpp"
#include <memory>

class Window;
class Shader;

class IRenderSystem {
public:
  virtual ~IRenderSystem() = default;
  virtual void render(entt::registry& reg) = 0;
};

class RendererSystem : public IRenderSystem {
public:
  RendererSystem(Window& window);
  ~RendererSystem() override;

  void render(entt::registry& reg) override;

  float gridSpacing  = 1.0f;
  float gridLineWidth = 0.008f;

private:
  void init();
  void shutdown();
  void initEntityBuffers(entt::registry& reg);
  void renderGrid();

private:
  Window& m_window;
  std::unique_ptr<Shader> m_shader;
  std::unique_ptr<Shader> m_gridShader;

  unsigned int m_gridVAO = 0;
  unsigned int m_gridVBO = 0;
};