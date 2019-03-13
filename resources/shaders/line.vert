#version 330 core

layout (location = 0) in vec3 Position;

uniform vec3 scale;
uniform vec3 origin;
uniform vec2 screen;

void main()
{
    mat3 scale_mat = mat3(
        scale.x, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f,
        0.0f, 0.0f, scale.z
    );

    vec3 out_position = scale_mat * Position + origin;
    gl_Position = vec4(out_position, 1.0f);
}
