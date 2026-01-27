$input v_color0, v_normal, v_texcoord0

#include <bgfx_shader.sh>

uniform vec4 u_color;

SAMPLER2D(s_tex_color, 0);

void main()
{
    vec4 tex_color = texture2D(s_tex_color, v_texcoord0.xy);
    gl_FragColor = v_color0 * tex_color * u_color;
}
