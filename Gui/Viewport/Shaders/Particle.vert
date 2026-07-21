#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 ParticleColor;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
	ParticleColor = aColor;
	gl_Position = uProjection * uView * vec4(aPos, 1.0);
	gl_PointSize = 8.0;
}
