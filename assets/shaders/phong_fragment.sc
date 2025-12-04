$input v_color0, v_normal, v_texcoord0, v_pos

#include <bgfx_shader.sh>

uniform vec4 u_light_pos;
uniform vec4 u_light_color;
uniform vec4 u_model_color;
uniform vec4 u_view_pos;

SAMPLER2D(s_tex_color, 0);

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 light_dir = normalize(u_light_pos.xyz - v_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    float ambient_strength = 0.1;
    vec4 ambient = ambient_strength * u_light_color;

    vec4 diffuse = diff * u_light_color;

    float specular_strength = 0.5;
    vec3 view_dir = normalize(u_view_pos.xyz - v_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    float specular_term = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec4 specular = specular_strength * specular_term * u_light_color;

    gl_FragColor = (ambient + diffuse + specular) * u_model_color;
}
