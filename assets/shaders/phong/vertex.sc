$input a_position, a_normal, a_texcoord0, a_color0
$output v_color0, v_normal, v_texcoord0, v_pos

/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bgfx_shader.sh>

// Used to compute the proper normal for models with non-uniform scale.
uniform mat4 u_normal_mat;

void main()
{
    vec4 position = vec4(a_position, 1.0);
    gl_Position = mul(u_modelViewProj, position);
    v_pos = mul(u_model[0], position);
    v_texcoord0 = a_texcoord0;
    v_normal = mul(u_normal_mat, vec4(a_normal, 0.0));
    v_color0 = a_color0;
}
