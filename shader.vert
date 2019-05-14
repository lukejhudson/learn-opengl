#version 330 core

layout (location = 0) in vec3 aPos;   // 3 floats of coordinates
layout (location = 1) in vec2 aTexCoord; // 2 floats of texture coordinates
layout (location = 2) in vec3 aNormal;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    //gl_Position = vec4(aPos, 1.0);

    // Matrix multiplication applied in reverse order
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoords = aTexCoord;

    // Transform normal to world space
    // w = 0.0 to remove translations
    Normal = vec3(model * vec4(aNormal, 0.0));
    // To prevent non-uniform scaling errors, instead use following, but much more expensive
    //Normal = mat3(transpose(inverse(model))) * aNormal; // IDE doesn't like "inverse" for some reason, but still compiles
    FragPos = vec3(model * vec4(aPos, 1.0));
}
