#version 130

in vec3 vtxPos;
in vec3 vtxCol;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 color;

void main()
{
	gl_Position = proj * view * model * vec4(vtxPos, 1.0);
	color = vec4(vtxCol, 1.0);
}
