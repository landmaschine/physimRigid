#pragma once

#include "entt/entity/fwd.hpp"
class Window;

class IRenderSystem {
public:
  ~IRenderSystem() = default;
  virtual void render(entt::registry& reg) = 0;
};

class RendererSystem : IRenderSystem {
public:
  RendererSystem(Window& window) : m_window(window) {}
  ~RendererSystem() {}

  void init();
  void render(entt::registry& reg) override;
  void shutdown();

private:
  Window& m_window;
};