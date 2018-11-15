#version 330 

in vec2 Texcoord;
out vec4 Color;

uniform sampler2D tex;

void main()
{
    Color = texture(tex, Texcoord);
}
