#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 2) in vec2 Uv;

out vec2 Texcoord;

uniform vec2 screen;
uniform vec3 camera_pos;
uniform mat3 camera_orientation; // inverse

void main()
{
    float near = 0.1f;
    float far = 100.0f;
    float scale = 0.1f;

    float aspect = screen.y / screen.x;
     
    mat3 scalemat = mat3(
        10.0f, 0.0f, 0.0f,
        0.0f, 10.0f, 0.0f,
        0.0f, 0.0f, 10.0f
    );

    mat4 perspective = mat4(
        near * aspect / scale, 0.0f, 0.0f, 0.0f,
        0.0f, near / scale, 0.0f, 0.0f,
        0.0f, 0.0f, -(near + far) / (far - near), -1,
        0.0f, 0.0f, -2 * near * far / (far - near), 0.0f
    );

    vec3 out_position = camera_orientation * scalemat * Position;
    gl_Position = perspective * vec4(out_position, 1.0f);
    Texcoord = Uv;
}
