#include "core.hpp"
#include "logger/logger.hpp"

#include "GLFW/glfw3.h"

void Core::init() {
  m_window.init(600, 800, "RigidSim");

  m_running = true;
}

void Core::shutdown() {

}

void Core::run() {
  while(m_running == true) {
    m_window.pollEvents();

    
  }
}
