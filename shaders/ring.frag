#version 460 core

in vec3 vWorldPosition;
in vec3 vNormal;

uniform vec3 uBaseColor;
uniform vec3 uSunPosition;
uniform bool uEmissive;

layout(location = 0) out vec4 fragColor;

void main()
{
    float diffuse = max(abs(dot(normalize(vNormal), normalize(uSunPosition - vWorldPosition))), 0.12);
    fragColor = vec4(uBaseColor * diffuse, 0.48);
}
