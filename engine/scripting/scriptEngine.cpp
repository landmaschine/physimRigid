#include "scriptEngine.hpp"

#include "ecs/ecs.hpp"
#include "components/components.hpp"
#include "Input/input.hpp"
#include "physics/inertia.hpp"
#include "logger/logger.hpp"

#include <glm/glm.hpp>

void ScriptEngine::init(Scene& scene) {
  m_scene = &scene;

  m_lua.open_libraries(
    sol::lib::base, sol::lib::math, sol::lib::string,
    sol::lib::table, sol::lib::io, sol::lib::os
  );

  bindMath();
  bindComponents();
  bindECS();

  m_lua["scene"] = m_scene;

  LOG("ScriptEngine initialised");
}

void ScriptEngine::loadScript(const std::string& path) {
  auto result = m_lua.safe_script_file(path, sol::script_pass_on_error);
  if (!result.valid()) {
    sol::error err = result;
    ERRLOG("Lua load '", path, "': ", err.what());
  } else {
    LOG("Loaded script: ", path);
  }
}

void ScriptEngine::callOnInit() {
  sol::protected_function fn = m_lua["on_init"];
  if (!fn.valid()) return;
  auto result = fn();
  if (!result.valid()) {
    sol::error err = result;
    ERRLOG("Lua on_init: ", err.what());
  }
}

void ScriptEngine::callOnUpdate(float dt) {
  sol::protected_function fn = m_lua["on_update"];
  if (!fn.valid()) return;
  auto result = fn(dt);
  if (!result.valid()) {
    sol::error err = result;
    ERRLOG("Lua on_update: ", err.what());
  }
}

void ScriptEngine::bindMath() {
  m_lua.new_usertype<glm::vec2>("vec2",
    sol::constructors<
      glm::vec2(),
      glm::vec2(float),
      glm::vec2(float, float)
    >(),
    "x", &glm::vec2::x,
    "y", &glm::vec2::y,

    sol::meta_function::addition,
      [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
    sol::meta_function::subtraction,
      [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
    sol::meta_function::multiplication, sol::overload(
      [](const glm::vec2& a, float b)  { return a * b; },
      [](float a, const glm::vec2& b)  { return a * b; }
    ),
    sol::meta_function::unary_minus,
      [](const glm::vec2& v) { return -v; },
    sol::meta_function::to_string,
      [](const glm::vec2& v) {
        return "vec2(" + std::to_string(v.x) + ", "
                       + std::to_string(v.y) + ")";
      },

    "length", [](const glm::vec2& v) { return glm::length(v); },
    "normalized", [](const glm::vec2& v) {
      float l = glm::length(v);
      return l > 0.0f ? v / l : glm::vec2{0.0f};
    }
  );

  m_lua.new_usertype<glm::vec4>("vec4",
    sol::constructors<
      glm::vec4(),
      glm::vec4(float),
      glm::vec4(float, float, float, float)
    >(),
    "x", &glm::vec4::x, "y", &glm::vec4::y,
    "z", &glm::vec4::z, "w", &glm::vec4::w,
    "r", &glm::vec4::r, "g", &glm::vec4::g,
    "b", &glm::vec4::b, "a", &glm::vec4::a
  );

  m_lua.set_function("color",
    [](float r, float g, float b, sol::optional<float> a) {
      return glm::vec4(r, g, b, a.value_or(1.0f));
    }
  );
}

void ScriptEngine::bindComponents() {

  m_lua.new_usertype<TagComponent>("TagComponent",
    "name", &TagComponent::name
  );

  m_lua.new_usertype<TransformComponent>("TransformComponent",
    "position", &TransformComponent::position,
    "scale",    &TransformComponent::scale,
    "rotation", &TransformComponent::rotation
  );

  m_lua.new_usertype<SpriteComponent>("SpriteComponent",
    "color",      &SpriteComponent::color,
    "size",       &SpriteComponent::size,
    "sort_order", &SpriteComponent::sortOrder
  );

  m_lua.new_usertype<CameraComponent>("CameraComponent",
    "ortho_size", &CameraComponent::orthoSize,
    "primary",    &CameraComponent::primary
  );

  // ── physics ──────────────────────────────────────────
  m_lua.new_usertype<RigidBody2D>("RigidBody2D",
    "velocity",         &RigidBody2D::velocity,
    "acceleration",     &RigidBody2D::acceleration,
    "force",            &RigidBody2D::force,
    "mass", sol::property(
      [](const RigidBody2D& rb) { return rb.mass; },
      [](RigidBody2D& rb, float m) { rb.setMass(m); }
    ),
    "restitution",      &RigidBody2D::restitution,
    "friction",         &RigidBody2D::friction,
    "angular_velocity", &RigidBody2D::angularVelocity,
    "torque",           &RigidBody2D::torque,
    "is_static", sol::property(
      [](const RigidBody2D& rb) { return rb.isStatic; },
      [](RigidBody2D& rb, bool s) { rb.setStatic(s); }
    ),
    "add_force", &RigidBody2D::addForce
  );

  m_lua.new_usertype<CircleCollider>("CircleCollider",
    "radius", &CircleCollider::radius,
    "offset", &CircleCollider::offset
  );

  m_lua.new_usertype<BoxCollider>("BoxCollider",
    "half_extents", &BoxCollider::halfExtents,
    "offset",       &BoxCollider::offset
  );

  m_lua.new_usertype<ConvexCollider>("ConvexCollider",
    "offset", &ConvexCollider::offset
  );
}

void ScriptEngine::bindECS() {

  m_lua.new_usertype<Entity>("Entity",
    "set_tag", [](Entity& e, const std::string& name) {
      if (e.hasComponent<TagComponent>())
        e.getComponent<TagComponent>().name = name;
      else
        e.addComponent<TagComponent>(name);
    },
    "tag", [](Entity& e) -> std::string {
      return e.hasComponent<TagComponent>()
        ? e.getComponent<TagComponent>().name : "";
    },

    "transform", [](Entity& e) -> TransformComponent& {
      if (!e.hasComponent<TransformComponent>())
        e.addComponent<TransformComponent>();
      return e.getComponent<TransformComponent>();
    },
    "set_position", [](Entity& e, float x, float y) {
      if (!e.hasComponent<TransformComponent>())
        e.addComponent<TransformComponent>();
      e.getComponent<TransformComponent>().position = {x, y};
    },
    "get_position", [](Entity& e) -> glm::vec2 {
      return e.hasComponent<TransformComponent>()
        ? e.getComponent<TransformComponent>().position : glm::vec2{0};
    },
    "set_scale", [](Entity& e, float x, float y) {
      if (!e.hasComponent<TransformComponent>())
        e.addComponent<TransformComponent>();
      e.getComponent<TransformComponent>().scale = {x, y};
    },
    "set_rotation", [](Entity& e, float r) {
      if (!e.hasComponent<TransformComponent>())
        e.addComponent<TransformComponent>();
      e.getComponent<TransformComponent>().rotation = r;
    },

    "sprite", [](Entity& e) -> SpriteComponent& {
      if (!e.hasComponent<SpriteComponent>())
        e.addComponent<SpriteComponent>();
      return e.getComponent<SpriteComponent>();
    },
    "set_sprite", [](Entity& e, float r, float g, float b, float a,
                     float w, float h, sol::optional<int> order) {
      if (!e.hasComponent<SpriteComponent>())
        e.addComponent<SpriteComponent>();
      auto& s     = e.getComponent<SpriteComponent>();
      s.color     = {r, g, b, a};
      s.size      = {w, h};
      s.sortOrder = order.value_or(0);
    },

    "camera", [](Entity& e) -> CameraComponent& {
      if (!e.hasComponent<CameraComponent>())
        e.addComponent<CameraComponent>();
      return e.getComponent<CameraComponent>();
    },
    "set_camera", [](Entity& e, float sz, sol::optional<bool> primary) {
      if (!e.hasComponent<CameraComponent>())
        e.addComponent<CameraComponent>();
      auto& c   = e.getComponent<CameraComponent>();
      c.orthoSize = sz;
      c.primary   = primary.value_or(true);
    },

    "rigidbody", [](Entity& e) -> RigidBody2D& {
      if (!e.hasComponent<RigidBody2D>())
        e.addComponent<RigidBody2D>();
      return e.getComponent<RigidBody2D>();
    },
    "set_rigidbody", [this](Entity& e, sol::table t) {
      if (!e.hasComponent<RigidBody2D>())
        e.addComponent<RigidBody2D>();
      auto& rb = e.getComponent<RigidBody2D>();
      if (t["mass"].valid())        rb.setMass(t["mass"]);
      if (t["restitution"].valid()) rb.restitution = t["restitution"];
      if (t["friction"].valid())    rb.friction    = t["friction"];
      if (t["is_static"].valid())   rb.setStatic(t["is_static"].get<bool>());
      if (t["linear_damping"].valid())  rb.linearDamping  = t["linear_damping"];
      if (t["angular_damping"].valid()) rb.angularDamping = t["angular_damping"];
      computeBodyInertia(m_scene->getRegistry(), static_cast<entt::entity>(e));
    },

    "circle_collider", [](Entity& e) -> CircleCollider& {
      if (!e.hasComponent<CircleCollider>())
        e.addComponent<CircleCollider>();
      return e.getComponent<CircleCollider>();
    },
    "set_circle_collider", [this](Entity& e, float radius) {
      if (!e.hasComponent<CircleCollider>())
        e.addComponent<CircleCollider>();
      e.getComponent<CircleCollider>().radius = radius;
      computeBodyInertia(m_scene->getRegistry(), static_cast<entt::entity>(e));
    },

    "box_collider", [](Entity& e) -> BoxCollider& {
      if (!e.hasComponent<BoxCollider>())
        e.addComponent<BoxCollider>();
      return e.getComponent<BoxCollider>();
    },
    "set_box_collider", [this](Entity& e, float hw, float hh) {
      if (!e.hasComponent<BoxCollider>())
        e.addComponent<BoxCollider>();
      e.getComponent<BoxCollider>().halfExtents = {hw, hh};
      computeBodyInertia(m_scene->getRegistry(), static_cast<entt::entity>(e));
    },

    "set_polygon_collider", [this](Entity& e, sol::table verts) {
      if (!e.hasComponent<ConvexCollider>())
        e.addComponent<ConvexCollider>();
      auto& cv = e.getComponent<ConvexCollider>();
      cv.vertices.clear();
      for (size_t i = 1; i <= verts.size(); ++i) {
        sol::table v = verts[i];
        cv.vertices.push_back({v[1].get<float>(), v[2].get<float>()});
      }
      cv.ensureCCW();
      computeBodyInertia(m_scene->getRegistry(), static_cast<entt::entity>(e));
    },

    "set_regular_polygon", [this](Entity& e, int sides, float radius) {
      if (sides < 3) sides = 3;
      if (!e.hasComponent<ConvexCollider>())
        e.addComponent<ConvexCollider>();
      auto& cv = e.getComponent<ConvexCollider>();
      cv.vertices.clear();
      for (int i = 0; i < sides; ++i) {
        float angle = 2.0f * 3.14159265f * static_cast<float>(i) / static_cast<float>(sides)
                    - 3.14159265f / 2.0f; // point up
        cv.vertices.push_back({radius * std::cos(angle), radius * std::sin(angle)});
      }
      cv.ensureCCW();
      computeBodyInertia(m_scene->getRegistry(), static_cast<entt::entity>(e));
    },

    "has_rigidbody", [](Entity& e) { return e.hasComponent<RigidBody2D>(); },
    "has_sprite",    [](Entity& e) { return e.hasComponent<SpriteComponent>(); },
    "has_collider",  [](Entity& e) {
      return e.hasComponent<CircleCollider>()
          || e.hasComponent<BoxCollider>()
          || e.hasComponent<ConvexCollider>();
    },

    "set_velocity", [](Entity& e, float vx, float vy) {
      if (!e.hasComponent<RigidBody2D>())
        e.addComponent<RigidBody2D>();
      e.getComponent<RigidBody2D>().velocity = {vx, vy};
    },
    "get_velocity", [](Entity& e) -> glm::vec2 {
      return e.hasComponent<RigidBody2D>()
        ? e.getComponent<RigidBody2D>().velocity : glm::vec2{0};
    },
    "apply_force", [](Entity& e, float fx, float fy) {
      if (e.hasComponent<RigidBody2D>())
        e.getComponent<RigidBody2D>().addForce({fx, fy});
    }
  );

  m_lua.new_usertype<Scene>("Scene",

    "create", [](Scene& s, sol::optional<std::string> name) -> Entity {
      auto e = s.createEntity();
      if (name.has_value()) {
        e.addComponent<TagComponent>(name.value());
        e.addComponent<TransformComponent>();
      }
      return e;
    },

    "find", [this](Scene& s, const std::string& name) -> sol::object {
      auto& reg = s.getRegistry();
      auto view = reg.view<TagComponent>();
      for (auto [entity, tag] : view.each()) {
        if (tag.name == name)
          return sol::make_object(m_lua, Entity(entity, &s));
      }
      return sol::lua_nil;
    },

    "destroy", [](Scene& s, Entity& e) {
      s.getRegistry().destroy(static_cast<entt::entity>(e));
    }
  );
}

void ScriptEngine::bindInput(InputSystem& input) {
  m_lua.new_usertype<InputSystem>("InputSystem",
    "mouse_down",     &InputSystem::mouseDown,
    "mouse_pressed",  &InputSystem::mousePressed,
    "mouse_released", &InputSystem::mouseReleased,
    "mouse_world",    &InputSystem::mouseWorld,
    "mouse_screen",   &InputSystem::mouseScreen,
    "is_key_down",    &InputSystem::isKeyDown,
    "is_key_pressed", &InputSystem::isKeyPressed,
    "is_key_released",&InputSystem::isKeyReleased
  );

  m_lua["input"] = &input;

  m_lua["KEY_SPACE"] = 32;
  m_lua["KEY_W"] = 87;  m_lua["KEY_A"] = 65;
  m_lua["KEY_S"] = 83;  m_lua["KEY_D"] = 68;
  m_lua["KEY_Q"] = 81;  m_lua["KEY_E"] = 69;
  m_lua["KEY_R"] = 82;  m_lua["KEY_F"] = 70;
  m_lua["KEY_UP"]    = 265; m_lua["KEY_DOWN"]  = 264;
  m_lua["KEY_LEFT"]  = 263; m_lua["KEY_RIGHT"] = 262;
  m_lua["KEY_1"] = 49; m_lua["KEY_2"] = 50;
  m_lua["KEY_3"] = 51; m_lua["KEY_4"] = 52;
}
