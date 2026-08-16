#version 460 core

in vec3 vWorldPosition;
in vec3 vNormal;

uniform vec3 uBaseColor;
uniform vec3 uSunPosition;
uniform bool uEmissive;

layout(location = 0) out vec4 fragColor;

void main()
{
    if (uEmissive) {
        fragColor = vec4(uBaseColor * 1.18, 1.0);
        return;
    }
    vec3 lightDirection = normalize(uSunPosition - vWorldPosition);
    float diffuse = max(dot(normalize(vNormal), lightDirection), 0.0);
    fragColor = vec4(uBaseColor * (0.055 + 0.945 * diffuse), 1.0);
}

