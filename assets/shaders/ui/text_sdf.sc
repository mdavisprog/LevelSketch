$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_font, 0);

void main()
{
    float alpha = texture2D(s_font, v_texcoord0.xy).r;
    if (alpha >= 0.25) {
        gl_FragColor = vec4(v_color0.rgb, alpha);
    } else {
        gl_FragColor = vec4_splat(0.0);
    }
}
