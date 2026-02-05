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

private:
  void init();
  void shutdown();
  void initEntityBuffers(entt::registry& reg);

private:
  Window& m_window;
  std::unique_ptr<Shader> m_shader;
};