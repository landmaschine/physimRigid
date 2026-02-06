#version 460 core

layout (location = 0) in vec2 aPos;

out vec2 v_worldPos;

uniform mat4 u_projection;

void main() {
  v_worldPos = aPos;
  gl_Position = u_projection * vec4(aPos, 0.0, 1.0);
}
