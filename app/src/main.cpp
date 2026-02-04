#include "entt/entt.hpp"
#include "renderer/renderSystem.hpp"
#include "renderer/window.hpp"
#include "core/core.hpp"
#include "Input/input.hpp"

int main(){
  entt::registry reg;

  Window window;
  RendererSystem renderer(window);
  InputSystem input(window);

  Core core(reg, window, renderer, input);
  core.init();
  core.run();

  return 0;
}