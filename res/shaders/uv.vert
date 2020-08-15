#version 330 core

in vec3 vtxPos;
in vec3 vtxNrm;
in vec2 vtxVal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 uv;
flat out vec3 nrm;

void main()
{
	gl_Position = proj * view * model * vec4(vtxPos, 1.0);

	uv = vtxVal;
	nrm = vtxNrm;
}
