#pragma once
#include <sol/sol.hpp>
#include <string>

class Scene;
class InputSystem;

class ScriptEngine {
public:
  ScriptEngine()  = default;
  ~ScriptEngine() = default;

  void init(Scene& scene);

  void bindInput(InputSystem& input);

  void loadScript(const std::string& path);

  void callOnInit();

  void callOnUpdate(float dt);

  sol::state& lua() { return m_lua; }

private:
  void bindMath();
  void bindComponents();
  void bindECS();

  sol::state m_lua;
  Scene*     m_scene = nullptr;
};
