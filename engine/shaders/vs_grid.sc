$input a_position
$output v_worldPos

#include <bgfx_shader.sh>

void main()
{
    v_worldPos = a_position.xy;
    gl_Position = mul(u_modelViewProj, vec4(a_position.xy, 0.0, 1.0));
}
