#pragma once

#include "entt/entt.hpp"
#include "scripting/scriptEngine.hpp"
#include "physics/physicsSystem.hpp"

class Scene;
class Window;
class RendererSystem;
class InputSystem;
class Timer;

enum class FrameRateMode {
  VSync,
  Limited,
  Unlimited
};

class Core {
public:
  Core(Scene& scene,
       PhysicsWorld& physics,
       Timer& timer,
       Window& window,
       RendererSystem& renderer,
       InputSystem& input);

  ~Core() { shutdown(); }

  void loadScript(const std::string& path);

  PhysicsWorld& physics()   { return m_physicsWorld; }
  ScriptEngine& scripting() { return m_scriptEngine; }

  void run();
  void setFrameRateMode(FrameRateMode mode);
  void setTargetFPS(double fps);

private:
  void shutdown();
  void sleepUntilTarget(float frameElapsedSeconds);

private:
  bool m_running = false;

  FrameRateMode m_frameRateMode = FrameRateMode::VSync;
  double m_targetFPS       = 60.0;
  double m_targetFrameTime = 1.0 / 60.0;

  Scene&         m_scene;
  ScriptEngine   m_scriptEngine;
  PhysicsWorld&  m_physicsWorld;

  Timer&          m_timer;
  Window&         m_window;
  RendererSystem& m_renderSystem;
  InputSystem&    m_input;
};
