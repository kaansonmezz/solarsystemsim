#version 460 core

in vec3 vColor;
uniform float uAlpha;
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(vColor, uAlpha);
}
