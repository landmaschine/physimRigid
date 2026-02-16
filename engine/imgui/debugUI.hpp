#pragma once

class PhysicsWorld;
class ConstraintSolverSystem;
class Scene;

class DebugUI {
public:
  void update(float dt, PhysicsWorld& physics, Scene& scene);

  bool visible = true;

private:
  float m_fpsAccum   = 0.f;
  int   m_fpsFrames  = 0;
  float m_displayFps = 0.f;
  float m_displayMs  = 0.f;
  
  static constexpr int kHistorySize = 120;
  float m_ftHistory[kHistorySize] = {};
  int   m_ftIndex = 0;
};
