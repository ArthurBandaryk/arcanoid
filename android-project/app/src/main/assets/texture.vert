#version 320 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texture;
uniform mat3 u_matrix;
out vec4 v_color;
out vec2 v_texture;

void main()
{
    v_color = a_color;
    v_texture = a_texture;
    vec3 result_pos = u_matrix * vec3(a_position, 1.0);
    gl_Position = vec4(result_pos, 1.0);
}
