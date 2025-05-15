#version 450

layout(push_constant) uniform pc {
    vec4 data;
} pushConstants;

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor,1)*pushConstants.data;
}