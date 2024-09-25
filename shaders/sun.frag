#version 460 core

in vec3 normals;
in vec4 tangents;
in vec2 coordinates;

uniform sampler2D texture_0;

out vec4 FragColor;

void main() {
    FragColor = texture(texture_0, coordinates);
}
