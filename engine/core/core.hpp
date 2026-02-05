#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

class Window;
class RendererSystem;
class InputSystem;

class Core {
public:
  Core(entt::registry& registry,
       Window& window,
       RendererSystem& renderer,
       InputSystem& input) 
    : m_registry(registry), m_window(window), m_renderSystem(renderer), m_input(input) {
      m_running = true;
    };

  ~Core() {
    shutdown();
  };

  void run();

private:
  void shutdown();

private:
  bool m_running = false;

  entt::registry& m_registry;
  Window& m_window;
  RendererSystem& m_renderSystem;
  InputSystem& m_input;
};
