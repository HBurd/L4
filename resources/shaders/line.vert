#version 330 core

layout (location = 0) in vec3 Position;

uniform vec3 scale;
uniform vec3 origin;
uniform vec2 screen;
uniform vec3 camera_pos;
uniform mat3 camera_orientation; // inverse
uniform mat4 perspective;

void main()
{
    float aspect = screen.y / screen.x;

    mat3 scale_mat = mat3(
        scale.x * aspect, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f,
        0.0f, 0.0f, scale.z
    );

    vec3 out_position = camera_orientation * (origin - camera_pos);
    vec4 perspective_transformed = perspective * vec4(out_position, 1.0f);
    bool clip = perspective_transformed.z < -1.0f;
    perspective_transformed = (1 / perspective_transformed.w) * perspective_transformed;
    gl_Position = perspective_transformed + vec4(scale_mat * Position, 0.0f);
    if (!clip) gl_Position.z = -1.0f;
}
