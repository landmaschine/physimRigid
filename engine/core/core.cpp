#include "core.hpp"

#include "renderer/window.hpp"
#include "renderer/renderSystem.hpp"
#include "Input/input.hpp"

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
