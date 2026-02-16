#include "debugUI.hpp"

#include <imgui.h>
#include "ecs/ecs.hpp"
#include "physics/physicsSystem.hpp"
#include "physics/systems/constraintSolver.hpp"
#include "physics/systems/gravitySystem.hpp"
#include "physics/systems/mouseGrab.hpp"
#include "physics/contact.hpp"
#include "physics/collisionEvents.hpp"

void DebugUI::update(float dt, PhysicsWorld& physics, Scene& scene) {
  if (!visible) return;

  m_fpsAccum += dt;
  m_fpsFrames++;
  if (m_fpsAccum >= 0.5f) {
    m_displayFps = static_cast<float>(m_fpsFrames) / m_fpsAccum;
    m_displayMs  = (m_fpsAccum / static_cast<float>(m_fpsFrames)) * 1000.f;
    m_fpsAccum   = 0.f;
    m_fpsFrames  = 0;
  }

  m_ftHistory[m_ftIndex] = dt * 1000.f;
  m_ftIndex = (m_ftIndex + 1) % kHistorySize;

  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowBgAlpha(0.7f);

  if (ImGui::Begin("Performance", nullptr,
        ImGuiWindowFlags_NoCollapse)) {
    ImGui::Text("FPS: %.1f", m_displayFps);
    ImGui::Text("Frame: %.2f ms", m_displayMs);

    ImGui::PlotLines("##ft", m_ftHistory, kHistorySize, m_ftIndex,
                     nullptr, 0.f, 33.3f, ImVec2(0, 50));

    float physDt = physics.getFixedTimestep();
    ImGui::Text("Physics substep: %.1f Hz (%.3f ms)",
                1.f / physDt, physDt * 1000.f);

    ImGui::Separator();
    auto& reg = scene.getRegistry();
    ImGui::Text("Entities: %zu", static_cast<size_t>(reg.storage<entt::entity>().size()));

    if (reg.ctx().contains<CollisionEvents>()) {
      auto& events = reg.ctx().get<CollisionEvents>();
      ImGui::Text("Contacts: begin=%zu stay=%zu end=%zu",
                  events.beginContacts.size(),
                  events.stayContacts.size(),
                  events.endContacts.size());
    }

    if (reg.ctx().contains<ContactManager>()) {
      auto& cm = reg.ctx().get<ContactManager>();
      ImGui::Text("Contact constraints: %zu", cm.size());
    }
  }
  ImGui::End();

  ImGui::SetNextWindowPos(ImVec2(10, 170), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(300, 350), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowBgAlpha(0.7f);

  if (ImGui::Begin("Physics", nullptr,
        ImGuiWindowFlags_NoCollapse)) {
    if (ImGui::CollapsingHeader("Timestep", ImGuiTreeNodeFlags_DefaultOpen)) {
      float hz = 1.f / physics.getFixedTimestep();
      if (ImGui::SliderFloat("Substep Hz", &hz, 30.f, 480.f, "%.0f")) {
        physics.setFixedTimestep(1.f / hz);
      }
    }

    auto* gravity = physics.getSystem<GravitySystem>();
    if (gravity && ImGui::CollapsingHeader("Gravity", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::SliderFloat("X", &gravity->m_gravity.x, -20.f, 20.f, "%.2f");
      ImGui::SliderFloat("Y", &gravity->m_gravity.y, -30.f, 10.f, "%.2f");
      if (ImGui::Button("Reset##grav")) {
        gravity->m_gravity = { 0.f, -9.81f };
      }
    }
    
    auto* solver = physics.getSystem<ConstraintSolverSystem>();
    if (solver && ImGui::CollapsingHeader("Solver", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::SliderInt("Velocity Iters", &solver->velocityIterations, 1, 50);
      ImGui::SliderInt("Position Iters", &solver->positionIterations, 1, 20);
      ImGui::SliderFloat("Baumgarte", &solver->baumgarte, 0.f, 1.f, "%.3f");
      ImGui::SliderFloat("Slop (m)", &solver->slop, 0.f, 0.05f, "%.4f");
      ImGui::SliderFloat("Max Pos Correction", &solver->maxPositionCorrection,
                         0.01f, 1.f, "%.3f");
      ImGui::SliderFloat("Restitution Threshold", &solver->restitutionThreshold,
                         0.f, 5.f, "%.2f m/s");
    }

    auto* grab = physics.getSystem<MouseGrabSystem>();
    if (grab && ImGui::CollapsingHeader("Mouse Grab")) {
      ImGui::SliderFloat("Frequency (Hz)", &grab->frequency, 0.5f, 20.f, "%.1f");
      ImGui::SliderFloat("Damping Ratio", &grab->dampingRatio, 0.f, 2.f, "%.2f");
      ImGui::SliderFloat("Max Force", &grab->maxForce, 10.f, 5000.f, "%.0f");
    }

    if (ImGui::CollapsingHeader("Systems")) {
      for (auto& sys : physics.systems()) {
        ImGui::Checkbox(sys->name(), &sys->enabled);
      }
    }
  }
  ImGui::End();
}
