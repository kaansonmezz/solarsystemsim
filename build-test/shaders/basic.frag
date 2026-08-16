#version 460 core

in vec3 vWorldPosition;
in vec3 vNormal;
in vec3 vColor;

uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(-uLightDirection);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 viewDirection = normalize(uCameraPosition - vWorldPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float specular = pow(max(dot(normal, halfwayDirection), 0.0), 48.0);
    vec3 color = vColor * (0.16 + 0.84 * diffuse) + vec3(0.25) * specular;
    fragColor = vec4(color, 1.0);
}

