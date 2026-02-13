#include "renderSystem.hpp"

#include "entt/entt.hpp"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glm/gtc/type_ptr.hpp"

#include "logger/logger.hpp"
#include "renderer/window.hpp"
#include "components/components.hpp"

#include <algorithm>
#include <cstring>

#include "vs_basic.sc.bin.h"
#include "fs_basic.sc.bin.h"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
  if (win) {
    win->setSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    bgfx::reset(static_cast<uint32_t>(width), static_cast<uint32_t>(height),
                BGFX_RESET_VSYNC);
  }
}

static bgfx::ShaderHandle loadShader(const uint8_t* data, uint32_t size) {
  const bgfx::Memory* mem = bgfx::copy(data, size);
  return bgfx::createShader(mem);
}

struct SpriteVertex {
  float x, y;
  float r, g, b, a;
};

RendererSystem::RendererSystem(Window& window) : m_window(window) {
  init();
}

RendererSystem::~RendererSystem() {
  shutdown();
}

void RendererSystem::init() {
  bgfx::renderFrame();

  bgfx::Init bgfxInit;
  bgfxInit.type = bgfx::RendererType::Vulkan;

  bgfxInit.platformData.nwh = m_window.getNativeHandle();
  bgfxInit.platformData.ndt = m_window.getNativeDisplayHandle();

  bgfxInit.resolution.width  = m_window.width();
  bgfxInit.resolution.height = m_window.height();
  bgfxInit.resolution.reset  = BGFX_RESET_VSYNC;

  if (!bgfx::init(bgfxInit)) {
    ERRLOG("Failed to initialise bgfx");
    std::exit(-1);
  }

  LOG("bgfx initialised renderer: ",
      bgfx::getRendererName(bgfx::getRendererType()));

  glfwSetFramebufferSizeCallback(m_window.getHandle(), framebuffer_size_callback);

  m_spriteLayout
    .begin()
    .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Float, true)
    .end();

  {
    bgfx::ShaderHandle vs = loadShader(vs_basic_sc_bin_h, sizeof(vs_basic_sc_bin_h));
    bgfx::ShaderHandle fs = loadShader(fs_basic_sc_bin_h, sizeof(fs_basic_sc_bin_h));
    m_spriteProgram = bgfx::createProgram(vs, fs, true);
  }
  bgfx::setViewClear(kViewGrid, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                      0x000000ff, 1.0f, 0);
}


void RendererSystem::renderSprites(entt::registry& reg,
                                    const glm::mat4& viewProj) {
  auto spriteView = reg.view<TransformComponent, SpriteComponent>();
  if (spriteView.size_hint() == 0) return;

  enum class ShapeType { Box, Circle, Convex };

  struct DrawCmd {
    const TransformComponent* tf;
    const SpriteComponent*    sp;
    ShapeType                 shape;
    float                     circleRadius;
    const std::vector<glm::vec2>* convexVerts;
  };

  std::vector<DrawCmd> drawList;
  drawList.reserve(spriteView.size_hint());

  for (auto e : spriteView) {
    DrawCmd cmd;
    cmd.tf          = &spriteView.get<TransformComponent>(e);
    cmd.sp          = &spriteView.get<SpriteComponent>(e);
    cmd.circleRadius = 0.0f;
    cmd.convexVerts  = nullptr;

    if (auto* cc = reg.try_get<CircleCollider>(e)) {
      cmd.shape        = ShapeType::Circle;
      cmd.circleRadius = cc->radius;
    } else if (auto* pc = reg.try_get<ConvexCollider>(e)) {
      cmd.shape       = ShapeType::Convex;
      cmd.convexVerts = &pc->vertices;
    } else {
      cmd.shape = ShapeType::Box;
    }

    drawList.push_back(cmd);
  }

  std::sort(drawList.begin(), drawList.end(),
            [](const DrawCmd& a, const DrawCmd& b) {
              return a.sp->sortOrder < b.sp->sortOrder;
            });

  uint32_t totalVerts   = 0;
  uint32_t totalIndices = 0;

  for (const auto& cmd : drawList) {
    switch (cmd.shape) {
      case ShapeType::Circle:
        totalVerts   += kCircleSegments + 1;
        totalIndices += kCircleSegments * 3;  
        break;
      case ShapeType::Convex:
        if (cmd.convexVerts && cmd.convexVerts->size() >= 3) {
          uint32_t n = static_cast<uint32_t>(cmd.convexVerts->size());
          totalVerts   += n + 1;             
          totalIndices += n * 3;           
        }
        break;
      case ShapeType::Box:
        totalVerts   += kVertsPerSprite;
        totalIndices += kIndicesPerSprite;
        break;
    }
  }

  if (totalVerts == 0) return;

  float viewMtx[16], projMtx[16];
  bx::mtxIdentity(viewMtx);

  float aspect = static_cast<float>(m_window.width()) /
                 static_cast<float>(m_window.height());

  float orthoSize = 5.0f;
  glm::vec2 camPos{0.0f};
  {
    auto camView = reg.view<TransformComponent, CameraComponent>();
    for (auto e : camView) {
      auto& cam = camView.get<CameraComponent>(e);
      if (cam.primary) {
        orthoSize = cam.orthoSize;
        camPos    = camView.get<TransformComponent>(e).position;
        break;
      }
    }
  }
  float hw = orthoSize * aspect;
  float hh = orthoSize;

  {
    float tx[16];
    bx::mtxTranslate(tx, -camPos.x, -camPos.y, 0.0f);
    std::memcpy(viewMtx, tx, sizeof(viewMtx));
  }
  bx::mtxOrtho(projMtx, -hw, hw, -hh, hh,
               -1.0f, 1.0f, 0.0f, bgfx::getCaps()->homogeneousDepth);

  bgfx::setViewRect(kViewSprites, 0, 0, m_window.width(), m_window.height());
  bgfx::setViewTransform(kViewSprites, viewMtx, projMtx);

  bgfx::TransientVertexBuffer tvb;
  bgfx::TransientIndexBuffer  tib;

  if (bgfx::getAvailTransientVertexBuffer(totalVerts, m_spriteLayout) < totalVerts)
    return;
  if (bgfx::getAvailTransientIndexBuffer(totalIndices) < totalIndices)
    return;

  bgfx::allocTransientVertexBuffer(&tvb, totalVerts, m_spriteLayout);
  bgfx::allocTransientIndexBuffer(&tib, totalIndices);

  auto* verts   = reinterpret_cast<SpriteVertex*>(tvb.data);
  auto* indices = reinterpret_cast<uint16_t*>(tib.data);

  uint32_t vi = 0;  
  uint32_t ii = 0; 

  for (const auto& drawCmd : drawList) {
    const auto& tf = *drawCmd.tf;
    const auto& sp = *drawCmd.sp;

    float cosR = std::cos(tf.rotation);
    float sinR = std::sin(tf.rotation);

    auto xform = [&](float lx, float ly) -> SpriteVertex {
      float rx = cosR * lx - sinR * ly + tf.position.x;
      float ry = sinR * lx + cosR * ly + tf.position.y;
      return { rx, ry, sp.color.r, sp.color.g, sp.color.b, sp.color.a };
    };

    switch (drawCmd.shape) {
      case ShapeType::Circle: {
        float r = drawCmd.circleRadius * tf.scale.x;
        uint16_t centreIdx = static_cast<uint16_t>(vi);

        verts[vi++] = xform(0.0f, 0.0f);

        for (uint32_t s = 0; s < kCircleSegments; ++s) {
          float angle = 2.0f * 3.14159265f * static_cast<float>(s)
                        / static_cast<float>(kCircleSegments);
          verts[vi++] = xform(r * std::cos(angle), r * std::sin(angle));
        }

        for (uint32_t s = 0; s < kCircleSegments; ++s) {
          indices[ii++] = centreIdx;
          indices[ii++] = static_cast<uint16_t>(centreIdx + 1 + s);
          indices[ii++] = static_cast<uint16_t>(centreIdx + 1 + (s + 1) % kCircleSegments);
        }
        break;
      }

      case ShapeType::Convex: {
        const auto& pts = *drawCmd.convexVerts;
        uint32_t n = static_cast<uint32_t>(pts.size());
        if (n < 3) break;

        uint16_t centreIdx = static_cast<uint16_t>(vi);

        glm::vec2 centroid{0.0f};
        for (const auto& p : pts) centroid += p;
        centroid /= static_cast<float>(n);

        verts[vi++] = xform(centroid.x * tf.scale.x,
                            centroid.y * tf.scale.y);

        for (uint32_t s = 0; s < n; ++s) {
          verts[vi++] = xform(pts[s].x * tf.scale.x,
                              pts[s].y * tf.scale.y);
        }

        for (uint32_t s = 0; s < n; ++s) {
          indices[ii++] = centreIdx;
          indices[ii++] = static_cast<uint16_t>(centreIdx + 1 + s);
          indices[ii++] = static_cast<uint16_t>(centreIdx + 1 + (s + 1) % n);
        }
        break;
      }

      case ShapeType::Box: {
        float hw2 = sp.size.x * tf.scale.x * 0.5f;
        float hh2 = sp.size.y * tf.scale.y * 0.5f;

        uint16_t base = static_cast<uint16_t>(vi);
        verts[vi++] = xform(-hw2, -hh2);
        verts[vi++] = xform( hw2, -hh2);
        verts[vi++] = xform( hw2,  hh2);
        verts[vi++] = xform(-hw2,  hh2);

        indices[ii++] = base + 0;
        indices[ii++] = base + 1;
        indices[ii++] = base + 2;
        indices[ii++] = base + 0;
        indices[ii++] = base + 2;
        indices[ii++] = base + 3;
        break;
      }
    }
  }

  float identity[16];
  bx::mtxIdentity(identity);
  bgfx::setTransform(identity);

  bgfx::setVertexBuffer(0, &tvb);
  bgfx::setIndexBuffer(&tib);
  bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                 BGFX_STATE_BLEND_ALPHA | BGFX_STATE_MSAA);
  bgfx::submit(kViewSprites, m_spriteProgram);
}

void RendererSystem::render(entt::registry& reg) {
  bgfx::setViewClear(kViewGrid, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                      0x000000ff, 1.0f, 0);
  bgfx::setViewRect(kViewGrid, 0, 0, m_window.width(), m_window.height());
  bgfx::touch(kViewGrid);

  glm::mat4 viewProj{1.0f};
  renderSprites(reg, viewProj);

  bgfx::frame();
}

void RendererSystem::shutdown() {
  if (bgfx::isValid(m_spriteProgram)) bgfx::destroy(m_spriteProgram);

  bgfx::shutdown();
}