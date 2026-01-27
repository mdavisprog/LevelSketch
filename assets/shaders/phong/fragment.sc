$input v_color0, v_normal, v_texcoord0, v_pos

#include <bgfx_shader.sh>

uniform vec4 u_view_pos;
uniform vec4 u_light_position;
uniform vec4 u_light_ambient;
uniform vec4 u_light_diffuse;
uniform vec4 u_light_specular;

uniform vec4 u_diffuse;
uniform vec4 u_specular_shininess;

SAMPLER2D(s_diffuse, 0);
SAMPLER2D(s_specular, 1);

#define u_specular u_specular_shininess.xyz
#define u_shininess u_specular_shininess.w

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 light_dir = normalize(u_light_position.xyz - v_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    vec4 tex_color = texture2D(s_diffuse, v_texcoord0.xy);
    vec4 ambient = tex_color * u_light_ambient;
    vec4 diffuse = (diff * tex_color) * u_diffuse * u_light_diffuse;

    vec3 view_dir = normalize(u_view_pos.xyz - v_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);

    vec4 tex_color_specular = texture2D(s_specular, v_texcoord0.xy);
    float specular_term = pow(max(dot(view_dir, reflect_dir), 0.0), u_shininess);
    vec4 specular = (specular_term * tex_color_specular) * u_light_specular;

    gl_FragColor = (ambient + diffuse + specular);
}
