#pragma once

#include "renderer/window.hpp"

class Core {
public:
  Core() = default;
  ~Core() = default;

  void init();
  void shutdown();

  void run();

private:
  bool m_running = false;

  Window m_window;

};
