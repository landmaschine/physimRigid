#include "entt/entt.hpp"
#include "renderer/renderSystem.hpp"
#include "renderer/window.hpp"
#include "core/core.hpp"
#include "Input/input.hpp"
#include "components/components.hpp"

int main(){
  entt::registry reg;

  Window window(800, 600, "physim");
  RendererSystem renderer(window);
  InputSystem input(window);

  auto tri = reg.create();
  reg.emplace<TransformComponent>(tri, glm::vec2{-2.0f, 0.0f}, glm::vec2{0.0f, 0.0f});
  reg.emplace<RigidBodyComponent>(tri, std::make_unique<Triangle>());
  reg.emplace<RenderComponent>(tri, glm::vec3{0.2f, 0.8f, 0.4f});

  auto rect = reg.create();
  reg.emplace<TransformComponent>(rect, glm::vec2{2.0f, 0.0f}, glm::vec2{0.0f, 0.0f});
  reg.emplace<RigidBodyComponent>(rect, std::make_unique<Rectangle>());
  reg.emplace<RenderComponent>(rect, glm::vec3{0.8f, 0.3f, 0.2f});

  Core core(reg, window, renderer, input);
  core.run();

  return 0;
}
