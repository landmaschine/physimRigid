#include "imgui_impl_bgfx.hpp"

#include <imgui.h>
#include <bgfx/bgfx.h>
#include <cstring>
#include <algorithm>

#include "vs_imgui.sc.bin.h"
#include "fs_imgui.sc.bin.h"

namespace ImGuiBgfx {

static bgfx::VertexLayout     s_layout;
static bgfx::ProgramHandle    s_program     = BGFX_INVALID_HANDLE;
static bgfx::TextureHandle    s_fontTexture = BGFX_INVALID_HANDLE;
static bgfx::UniformHandle    s_texUniform  = BGFX_INVALID_HANDLE;
static bgfx::ViewId           s_viewId      = 255;

static bgfx::ShaderHandle loadShader(const uint8_t* data, uint32_t size) {
  const bgfx::Memory* mem = bgfx::copy(data, size);
  return bgfx::createShader(mem);
}

void init(bgfx::ViewId viewId) {
  s_viewId = viewId;
  
  s_layout
    .begin()
    .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
    .end();

  
  bgfx::ShaderHandle vs = loadShader(vs_imgui_sc_bin_h, sizeof(vs_imgui_sc_bin_h));
  bgfx::ShaderHandle fs = loadShader(fs_imgui_sc_bin_h, sizeof(fs_imgui_sc_bin_h));
  s_program = bgfx::createProgram(vs, fs, true);
  
  s_texUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

  
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

  s_fontTexture = bgfx::createTexture2D(
    static_cast<uint16_t>(width),
    static_cast<uint16_t>(height),
    false, 1,
    bgfx::TextureFormat::BGRA8,
    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
    BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
    bgfx::copy(pixels, width * height * 4));

  io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(s_fontTexture.idx)));
}

void shutdown() {
  if (bgfx::isValid(s_program))     bgfx::destroy(s_program);
  if (bgfx::isValid(s_fontTexture)) bgfx::destroy(s_fontTexture);
  if (bgfx::isValid(s_texUniform))  bgfx::destroy(s_texUniform);

  s_program     = BGFX_INVALID_HANDLE;
  s_fontTexture = BGFX_INVALID_HANDLE;
  s_texUniform  = BGFX_INVALID_HANDLE;
}

void renderDrawData(uint16_t fbWidth, uint16_t fbHeight) {
  ImDrawData* drawData = ImGui::GetDrawData();
  if (!drawData || drawData->CmdListsCount == 0)
    return;

  
  float L = drawData->DisplayPos.x;
  float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
  float T = drawData->DisplayPos.y;
  float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

  float ortho[16];
  std::memset(ortho, 0, sizeof(ortho));
  ortho[0]  =  2.0f / (R - L);
  ortho[5]  =  2.0f / (T - B);
  ortho[10] =  1.0f;
  ortho[12] = (L + R) / (L - R);
  ortho[13] = (T + B) / (B - T);
  ortho[15] =  1.0f;
  
  bgfx::setViewRect(s_viewId, 0, 0, fbWidth, fbHeight);
  bgfx::setViewMode(s_viewId, bgfx::ViewMode::Sequential);

  float identity[16];
  std::memset(identity, 0, sizeof(identity));
  identity[0] = identity[5] = identity[10] = identity[15] = 1.0f;

  bgfx::setViewTransform(s_viewId, identity, ortho);

  ImVec2 clipOff   = drawData->DisplayPos;
  ImVec2 clipScale = drawData->FramebufferScale;

  for (int n = 0; n < drawData->CmdListsCount; ++n) {
    const ImDrawList* cmdList = drawData->CmdLists[n];
    int numVerts   = cmdList->VtxBuffer.Size;
    int numIndices = cmdList->IdxBuffer.Size;

    if (numVerts == 0 || numIndices == 0)
      continue;
    if (bgfx::getAvailTransientVertexBuffer(numVerts, s_layout) < (uint32_t)numVerts)
      break;
    if (bgfx::getAvailTransientIndexBuffer(numIndices) < (uint32_t)numIndices)
      break;

    bgfx::TransientVertexBuffer tvb;
    bgfx::TransientIndexBuffer  tib;
    bgfx::allocTransientVertexBuffer(&tvb, numVerts, s_layout);
    bgfx::allocTransientIndexBuffer(&tib, numIndices);

    std::memcpy(tvb.data, cmdList->VtxBuffer.Data, numVerts * sizeof(ImDrawVert));
    std::memcpy(tib.data, cmdList->IdxBuffer.Data, numIndices * sizeof(ImDrawIdx));

    uint32_t offset = 0;
    for (int i = 0; i < cmdList->CmdBuffer.Size; ++i) {
      const ImDrawCmd& cmd = cmdList->CmdBuffer[i];

      if (cmd.UserCallback) {
        cmd.UserCallback(cmdList, &cmd);
        continue;
      }

      if (cmd.ElemCount == 0) continue;

      ImVec4 clipRect;
      clipRect.x = (cmd.ClipRect.x - clipOff.x) * clipScale.x;
      clipRect.y = (cmd.ClipRect.y - clipOff.y) * clipScale.y;
      clipRect.z = (cmd.ClipRect.z - clipOff.x) * clipScale.x;
      clipRect.w = (cmd.ClipRect.w - clipOff.y) * clipScale.y;

      if (clipRect.x < fbWidth && clipRect.y < fbHeight &&
          clipRect.z >= 0.0f   && clipRect.w >= 0.0f) {

        uint16_t sx = static_cast<uint16_t>(std::max(clipRect.x, 0.0f));
        uint16_t sy = static_cast<uint16_t>(std::max(clipRect.y, 0.0f));
        uint16_t sw = static_cast<uint16_t>(std::min(clipRect.z, (float)fbWidth)  - sx);
        uint16_t sh = static_cast<uint16_t>(std::min(clipRect.w, (float)fbHeight) - sy);
        bgfx::setScissor(sx, sy, sw, sh);

        bgfx::TextureHandle tex = s_fontTexture;
        if (cmd.GetTexID()) {
          uintptr_t id = reinterpret_cast<uintptr_t>(cmd.GetTexID());
          tex.idx = static_cast<uint16_t>(id);
        }
        bgfx::setTexture(0, s_texUniform, tex);
        bgfx::setState(
          BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
          BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                BGFX_STATE_BLEND_INV_SRC_ALPHA) |
          BGFX_STATE_MSAA);

        bgfx::setVertexBuffer(0, &tvb, cmd.VtxOffset, numVerts);
        bgfx::setIndexBuffer(&tib, offset, cmd.ElemCount);

        bgfx::submit(s_viewId, s_program);
      }

      offset += cmd.ElemCount;
    }
  }
}

} 
