$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_tex_color, 0);

void main()
{
    vec4 tex_color = texture2D(s_tex_color, v_texcoord0.xy);
    gl_FragColor = v_color0 * vec4(1, 1, 1, tex_color.r);
}
