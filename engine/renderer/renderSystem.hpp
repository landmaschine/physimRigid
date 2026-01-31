#pragma once
#include <cstdint>

class RendererSystem {
public:
  RendererSystem() = default;
  ~RendererSystem() {}

  void init();
  void shutdown();

  void setViewport();

private:
  uint32_t width;
  uint32_t height;

};