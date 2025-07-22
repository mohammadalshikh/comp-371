#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
out vec2 TexCoord;
void main() {
    TexCoord = aTexCoord;
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos, 1.0);
}
