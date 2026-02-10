$input v_color0, v_normal, v_texcoord0, v_pos

#include <bgfx_shader.sh>

uniform vec4 u_view_pos;

// Directional light properties.
uniform vec4 u_light_dir_direction;
uniform vec4 u_light_dir_ambient;
uniform vec4 u_light_dir_diffuse;
uniform vec4 u_light_dir_specular;

// Point light properties.
uniform vec4 u_light_point_position;
uniform vec4 u_light_point_ambient;
uniform vec4 u_light_point_diffuse;
uniform vec4 u_light_point_specular;
uniform vec4 u_light_point_properties;
#define u_light_point_constant u_light_point_properties.x
#define u_light_point_linear u_light_point_properties.y
#define u_light_point_quadratic u_light_point_properties.z

// Mesh properties
SAMPLER2D(s_diffuse, 0);
SAMPLER2D(s_specular, 1);
uniform vec4 u_specular_shininess;
#define u_specular u_specular_shininess.xyz
#define u_shininess u_specular_shininess.w

vec4 calcDirectionalLight(vec3 normal, vec3 view_dir, vec2 texcoord) {
    vec3 dir = normalize(u_light_dir_direction.xyz);
    float diff = max(dot(normal, dir), 0.0);
    vec3 reflect_dir = reflect(-dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_shininess);

    vec4 ambient = u_light_dir_ambient * texture2D(s_diffuse, texcoord);
    vec4 diffuse = u_light_dir_diffuse * diff * texture2D(s_diffuse, texcoord);
    vec4 specular = u_light_dir_specular * spec * texture2D(s_specular, texcoord);

    return ambient + diffuse + specular;
}

vec4 calcPointLight(vec3 normal, vec3 view_dir, vec3 frag_pos, vec2 texcoord) {
    vec3 dir = normalize(u_light_point_position.xyz - frag_pos);
    float diff = max(dot(normal, dir), 0.0);
    vec3 reflect_dir = reflect(-dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_shininess);

    float distance = length(u_light_point_position.xyz - frag_pos);
    float attenuation = 1.0 / (u_light_point_constant + u_light_point_linear * distance +
        u_light_point_quadratic * (distance * distance));

    vec4 ambient  = u_light_point_ambient * texture2D(s_diffuse, texcoord);
    vec4 diffuse  = u_light_point_diffuse * diff * texture2D(s_diffuse, texcoord);
    vec4 specular = u_light_point_specular * spec * texture2D(s_specular, texcoord);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main() {
    vec3 normal = normalize(v_normal);
    vec3 view_dir = normalize(u_view_pos.xyz - v_pos);

    vec4 result = vec4_splat(1.0);
    result = calcDirectionalLight(normal, view_dir, v_texcoord0.xy);
    result += calcPointLight(normal, view_dir, v_pos, v_texcoord0.xy);

    gl_FragColor = result;
}
