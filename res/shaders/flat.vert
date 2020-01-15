#version 330 core

in vec3 vtxPos;
in vec3 vtxCol;
in vec3 vtxNrm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

flat out vec3 color;

void main()
{
	gl_Position = proj * view * model * vec4(vtxPos, 1.0);

	color = vtxCol;
}
