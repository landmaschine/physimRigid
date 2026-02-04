#include "core.hpp"

#include "renderer/window.hpp"
#include "renderer/renderSystem.hpp"
#include "Input/input.hpp"

void Core::init() {
  m_window.init(800, 600, "RigidSim");
  m_renderSystem.init();

  m_running = true;
}

void Core::shutdown() {

}

void Core::run() {
  while(m_running == true) {
    if(m_window.shouldClose()) {
      m_running = false;
    }

    m_input.processInput(m_registry);

    m_renderSystem.render(m_registry);

    m_window.pollEvents();
    m_window.swapBuffers();

  }
}
