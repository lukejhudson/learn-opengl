#version 330 core
out vec4 FragColor;

void main() {
    // Colour the lamp pure white with no lighting effects (since it's the light source)
    FragColor = vec4(1.0);
}
