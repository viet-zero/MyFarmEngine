#version 330 core
layout(location = 0) in vec2 position;
uniform mat4 viewProjection;
layout(location = 1) in vec4 rectangle;
layout(location = 2) in vec4 uvRegion;
layout(location = 3) in vec4 tint;
out vec4 instanceTint;
out vec2 uv;
out vec2 localUv;
void main()
{
    gl_Position = viewProjection * vec4(rectangle.xy + position * rectangle.zw, 0.0, 1.0);
    uv = uvRegion.xy + position * uvRegion.zw;
    localUv = position;
    instanceTint = tint;
}
