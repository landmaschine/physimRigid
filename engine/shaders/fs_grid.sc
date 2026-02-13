$input v_worldPos

#include <bgfx_shader.sh>

uniform vec4 u_gridParams; // x = gridSpacing, y = lineWidth, z = unused, w = unused
uniform vec4 u_gridColor;  // rgb = color, a = unused

void main()
{
    float gridSpacing = u_gridParams.x;
    float lineWidth   = u_gridParams.y;

    vec2 grid = abs(fract(v_worldPos / gridSpacing + 0.5) - 0.5) * gridSpacing;
    float d = min(grid.x, grid.y);

    float fw = fwidth(d);
    float edge0 = lineWidth - fw;
    float edge1 = lineWidth + fw;
    float s = smoothstep(edge0, edge1, d);
    float lineFactor = 1.0 - s;

    if (lineFactor < 0.01)
        discard;

    gl_FragColor = vec4(u_gridColor.rgb, lineFactor * 0.6);
}
