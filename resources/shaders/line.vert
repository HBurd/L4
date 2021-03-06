#version 330 core

layout (location = 0) in vec3 Position;

uniform vec3 scale;
uniform mat3 rotation;
uniform vec3 origin;
uniform vec3 camera_pos;
uniform mat3 camera_orientation; // inverse
uniform mat4 perspective;

void main()
{
    mat3 scale_mat = mat3(
        scale.x, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f,
        0.0f, 0.0f, scale.z
    );

    vec3 out_position = camera_orientation * (rotation * scale_mat * Position + origin - camera_pos);
    gl_Position = perspective * vec4(out_position, 1.0f);
}
