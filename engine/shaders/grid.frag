#version 460 core

in vec2 v_worldPos;
out vec4 FragColor;

uniform float u_gridSpacing;   // distance between grid lines (world units)
uniform float u_lineWidth;     // line thickness in world units
uniform vec3  u_gridColor;     // color of grid lines

void main() {
  // Distance to nearest grid line on each axis
  vec2 grid = abs(fract(v_worldPos / u_gridSpacing + 0.5) - 0.5) * u_gridSpacing;
  float d = min(grid.x, grid.y);

  // Anti-aliased step
  float fw = fwidth(d);
  float line = 1.0 - smoothstep(u_lineWidth - fw, u_lineWidth + fw, d);

  if (line < 0.01) discard;

  FragColor = vec4(u_gridColor, line * 0.6);
}
