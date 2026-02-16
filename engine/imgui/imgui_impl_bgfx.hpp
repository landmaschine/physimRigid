#pragma once
#include <bgfx/bgfx.h>

namespace ImGuiBgfx {

void init(bgfx::ViewId viewId = 255);

void shutdown();

void renderDrawData(uint16_t width, uint16_t height);
} 
