#version 330 core
layout(location = 0) in vec2 position;
uniform mat4 viewProjection;
uniform vec4 rectangle;
uniform vec4 uvRegion;
out vec2 uv;
void main()
{
    gl_Position = viewProjection * vec4(rectangle.xy + position * rectangle.zw, 0.0, 1.0);
    uv = uvRegion.xy + position * uvRegion.zw;
}
