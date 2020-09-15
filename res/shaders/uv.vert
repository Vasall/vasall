#version 330 core

in vec3 vtxPos;
in vec3 vtxNrm;
in vec2 vtxVal;

uniform mat4 mpos;
uniform mat4 mrot;
uniform mat4 view;
uniform mat4 proj;

out vec2 uv;
flat out vec3 nrm;

void main()
{
	vec4 rotnrm;

	gl_Position = proj * view * mpos * mrot * vec4(vtxPos, 1.0);

	uv = vtxVal;
	
	rotnrm = mrot * vec4(vtxNrm, 1.0);
	nrm = rotnrm.xyz;
}
