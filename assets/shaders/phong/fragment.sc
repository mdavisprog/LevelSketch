$input v_color0, v_normal, v_texcoord0, v_pos

#include <bgfx_shader.sh>

uniform vec4 u_view_pos;

// Directional light properties.
uniform vec4 u_light_dir_direction;
uniform vec4 u_light_dir_ambient;
uniform vec4 u_light_dir_diffuse;
uniform vec4 u_light_dir_specular;

// Point light properties.
#define NUM_POINT_LIGHTS 4
uniform vec4 u_light_point_position[NUM_POINT_LIGHTS];
uniform vec4 u_light_point_ambient[NUM_POINT_LIGHTS];
uniform vec4 u_light_point_diffuse[NUM_POINT_LIGHTS];
uniform vec4 u_light_point_specular[NUM_POINT_LIGHTS];
uniform vec4 u_light_point_properties[NUM_POINT_LIGHTS];
#define u_light_point_constant(index) u_light_point_properties[index].x
#define u_light_point_linear(index) u_light_point_properties[index].y
#define u_light_point_quadratic(index) u_light_point_properties[index].z

// Light options.
uniform vec4 u_options;
#define num_lights u_options.x

// Mesh properties
SAMPLER2D(s_diffuse, 0);
SAMPLER2D(s_specular, 1);
uniform vec4 u_diffuse;
uniform vec4 u_specular_shininess;
#define u_specular u_specular_shininess.xyz
#define u_shininess u_specular_shininess.w

vec4 calcDirectionalLight(vec3 normal, vec3 view_dir, vec2 texcoord) {
    vec3 dir = normalize(u_light_dir_direction.xyz);
    float diff = max(dot(normal, dir), 0.0);
    vec3 reflect_dir = reflect(-dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_shininess);

    vec4 ambient = u_light_dir_ambient * texture2D(s_diffuse, texcoord) * u_diffuse;
    vec4 diffuse = u_light_dir_diffuse * diff * texture2D(s_diffuse, texcoord) * u_diffuse;
    vec4 specular = u_light_dir_specular * spec * texture2D(s_specular, texcoord) * vec4(u_specular, 1.0);

    return ambient + diffuse + specular;
}

vec4 calcPointLight(int index, vec3 normal, vec3 view_dir, vec3 frag_pos, vec2 texcoord) {
    vec3 dir = normalize(u_light_point_position[index].xyz - frag_pos);
    float diff = max(dot(normal, dir), 0.0);
    vec3 reflect_dir = reflect(-dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_shininess);

    float distance = length(u_light_point_position[index].xyz - frag_pos);
    float attenuation = 1.0 / (u_light_point_constant(index) + u_light_point_linear(index) * distance +
        u_light_point_quadratic(index) * (distance * distance));

    vec4 ambient  = u_light_point_ambient[index] * texture2D(s_diffuse, texcoord) * u_diffuse;
    vec4 diffuse  = u_light_point_diffuse[index] * diff * texture2D(s_diffuse, texcoord) * u_diffuse;
    vec4 specular = u_light_point_specular[index] * spec * texture2D(s_specular, texcoord) * vec4(u_specular, 1.0);

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

    int count = int(min(num_lights, NUM_POINT_LIGHTS));
    for (int i = 0; i < count; i += 1.0) {
        result += calcPointLight(i, normal, view_dir, v_pos, v_texcoord0.xy);
    }

    gl_FragColor = result;
}
