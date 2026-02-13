#include "renderer/renderSystem.hpp"
#include "renderer/window.hpp"
#include "core/core.hpp"
#include "Input/input.hpp"
#include "ecs/ecs.hpp"
#include "timer/timer.hpp"

#include "physics/systems/inertiaSystem.hpp"
#include "physics/systems/gravitySystem.hpp"
#include "physics/systems/collisionDetection.hpp"
#include "physics/systems/mouseGrab.hpp"
#include "physics/systems/constraintSolver.hpp"

int main() {
  Scene scene;
  Timer timer;

  Window window(1280, 720, "PhySim");
  RendererSystem renderer(window);
  InputSystem input(window);

  PhysicsWorld physics;

  physics.addSystem<InertiaSystem>();
  physics.addSystem<GravitySystem>(glm::vec2{0.0f, -9.81f});
  physics.addSystem<MouseGrabSystem>();
  physics.addSystem<CollisionDetectionSystem>();
  auto& solver = physics.addSystem<ConstraintSolverSystem>();
  solver.velocityIterations = 12;
  solver.positionIterations = 4;

  physics.setFixedTimestep(1.0f / 240.0f);

  Core core(scene, physics, timer, window, renderer, input);
  core.setFrameRateMode(FrameRateMode::VSync);

  core.loadScript("../../scripts/init.lua");

  core.run();
  return 0;
}
