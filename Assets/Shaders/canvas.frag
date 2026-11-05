#version 330 core
in vec2 uv;
in vec2 localUv;
uniform sampler2D image;
in vec4 instanceTint;
uniform int ellipseMask;
out vec4 color;
void main()
{
    color = texture(image, uv) * instanceTint;
    if(ellipseMask != 0)
    {
        vec2 centered=(localUv-vec2(0.5))*2.0;
        float coverage=1.0-smoothstep(0.82,1.0,dot(centered,centered));
        color.a*=coverage;
    }
}
