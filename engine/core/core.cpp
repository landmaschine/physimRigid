#include "core.hpp"

#include "ecs/ecs.hpp"
#include "components/components.hpp"
#include "renderer/window.hpp"
#include "renderer/renderSystem.hpp"
#include "Input/input.hpp"
#include "timer/timer.hpp"
#include "logger/logger.hpp"
#include "physics/pointerState.hpp"

#include <thread>
#include <algorithm>

Core::Core(Scene& scene,
           PhysicsWorld& physics,
           Timer& timer,
           Window& window,
           RendererSystem& renderer,
           InputSystem& input)
  : m_scene(scene), m_physicsWorld(physics), m_timer(timer),
    m_window(window), m_renderSystem(renderer), m_input(input)
{
  m_running = true;
  m_scriptEngine.init(m_scene);
  m_scriptEngine.bindInput(m_input);
}

void Core::shutdown() {
}

void Core::loadScript(const std::string& path) {
  m_scriptEngine.loadScript(path);
}

void Core::setFrameRateMode(FrameRateMode mode) {
  m_frameRateMode = mode;
}

void Core::setTargetFPS(double fps) {
  m_targetFPS       = fps;
  m_targetFrameTime = 1.0 / fps;
}

void Core::sleepUntilTarget(float frameElapsedSeconds) {
  if (m_frameRateMode != FrameRateMode::Limited) return;

  double remaining = m_targetFrameTime - static_cast<double>(frameElapsedSeconds);
  if (remaining > 0.0) {
    if (remaining > 0.002) {
      std::this_thread::sleep_for(
        std::chrono::microseconds(static_cast<long long>((remaining - 0.001) * 1'000'000)));
    }
    while (m_timer.elapsed<s>() < static_cast<float>(m_targetFrameTime)) {

    }
  }
}

void Core::run() {
  auto& reg = m_scene.getRegistry();

  m_physicsWorld.init(reg);
  m_scriptEngine.callOnInit();

  LOG("Engine running");
  m_timer.start();

  while (m_running) {
    float frameSeconds = m_timer.stop<s>();
    m_timer.start();

    float dt = std::min(frameSeconds, 0.25f);

    if (m_window.shouldClose()) {
      m_running = false;
    }

    m_window.pollEvents();

    {
      float aspect = static_cast<float>(m_window.width()) /
                     static_cast<float>(m_window.height());
      float orthoSize = 5.0f;
      glm::vec2 camPos{0.0f};
      auto camView = reg.view<TransformComponent, CameraComponent>();
      for (auto e : camView) {
        auto& cam = camView.get<CameraComponent>(e);
        if (cam.primary) {
          orthoSize = cam.orthoSize;
          camPos    = camView.get<TransformComponent>(e).position;
          break;
        }
      }
      m_input.worldHalfW = orthoSize * aspect;
      m_input.worldHalfH = orthoSize;
      m_input.cameraPos  = camPos;
    }

    m_input.processInput(dt);

    {
      auto& reg = m_scene.getRegistry();
      if (reg.ctx().contains<PointerState>()) {
        auto& ps = reg.ctx().get<PointerState>();
        ps.down     = m_input.mouseDown;
        ps.pressed  = m_input.mousePressed;
        ps.released = m_input.mouseReleased;
        ps.worldPos = m_input.mouseWorld;
      }
    }

    m_scriptEngine.callOnUpdate(dt);

    m_physicsWorld.update(reg, dt);

    m_renderSystem.render(reg);

    sleepUntilTarget(m_timer.elapsed<s>());
  }
}
