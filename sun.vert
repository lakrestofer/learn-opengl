#version 460 core

layout (location = 0) in vec3 in_vertices;
layout (location = 1) in vec3 in_normals;
layout (location = 2) in vec3 in_tangents;
layout (location = 3) in vec3 in_coordinates;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 normals;
out vec3 tangents;
out vec3 coordinates;
// out vec3 gl_Position 

void main() {
    normals  = in_normals;
    tangents = in_tangents;
    coord    = in_coord;
    gl_Position  = projection * view * model * vec4(vertices, 1.0);
}
