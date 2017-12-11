#version 150

in vec3 vColor;

out vec4 oFragColor; //write the final color into this variable

void main() {
    oFragColor = vec4(vColor, 0.0);
}
