#pragma once

#include "entt/entity/fwd.hpp"
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

class Window;

class RendererSystem {
public:
  RendererSystem(Window& window);
  ~RendererSystem();

  void render(entt::registry& reg);

private:
  void init();
  void shutdown();

  void renderSprites(entt::registry& reg, const glm::mat4& viewProj);

private:
  Window& m_window;

  bgfx::VertexLayout m_spriteLayout;

  bgfx::ProgramHandle m_spriteProgram = BGFX_INVALID_HANDLE;

  static constexpr bgfx::ViewId kViewGrid    = 0;
  static constexpr bgfx::ViewId kViewSprites = 1;

  static constexpr uint32_t kMaxSprites      = 4096;
  static constexpr uint32_t kVertsPerSprite  = 4;
  static constexpr uint32_t kIndicesPerSprite = 6;
  static constexpr uint32_t kCircleSegments  = 32;
};