#version 460 core

layout (location = 0) in vec3 vertices;
layout (location = 1) in vec3 normals;
layout (location = 2) in vec3 tangents;
layout (location = 3) in vec3 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// out vec3 gl_Position 

void main() {
    gl_Position = projection * view * model * vec4(vertices, 1.0);
}
