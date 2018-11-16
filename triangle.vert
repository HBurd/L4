#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

uniform mat3 rotation;
uniform vec3 origin;
uniform vec2 screen;
uniform vec3 camera_pos;
uniform mat3 camera_orientation; // inverse

out VS_OUTPUT
{
    vec3 Color;
} OUT;

void main()
{
    float flip = 3.1416f * 0.125f;

    mat3 vert_flip = mat3(
        1.0f, 0.0f, 0.0f,
        0.0f, cos(flip), sin(flip),
        0.0f, -sin(flip), cos(flip)
    );

    mat3 rotation_matrix = vert_flip * rotation;

    float near = 0.1f;
    float far = 100.0f;
    float scale = 0.07f;

    float aspect = screen.y / screen.x;

    mat4 perspective = mat4(
        near * aspect / scale, 0.0f, 0.0f, 0.0f,
        0.0f, near / scale, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -1,
        0.0f, 0.0f, -2 * near * far / (far - near), 0.0f
    );

    vec3 out_position = camera_orientation * (rotation_matrix * Position + origin - camera_pos);
    gl_Position = perspective * vec4(out_position, 1.0f);

    vec3 light_direction = vec3(1.0f, 0.0f, 0.0f);
    vec3 rotated_normal = rotation_matrix * Normal;
    OUT.Color = vec3(1.0f, 1.0f, 1.0f) * dot(rotated_normal, light_direction);
}
