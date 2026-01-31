#include "core.hpp"
#include "logger/logger.hpp"

#include "GLFW/glfw3.h"

void Core::init() {
  m_window.init(800, 600, "RigidSim");

  m_running = true;
}

void Core::shutdown() {

}

void Core::run() {
  while(m_running == true) {
    if(m_window.shouldClose()) {
      m_running = false;
    }
    m_window.pollEvents();

    
  }
}
