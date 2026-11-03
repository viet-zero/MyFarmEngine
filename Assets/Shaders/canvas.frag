#version 330 core
in vec2 uv;
uniform sampler2D image;
uniform vec4 tint;
out vec4 color;
void main()
{
    color = texture(image, uv) * tint;
}
