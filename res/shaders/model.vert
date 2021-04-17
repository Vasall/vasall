#version 420

layout(location=0) in vec3 vtxPos;
layout(location=1) in vec2 vtxTex;
layout(location=2) in vec3 vtxNrm;

layout(binding=0) uniform UBO {
	mat4 mpos;
	mat4 mrot;
	mat4 view;
	mat4 proj;
};

layout(location=0)out vec2 uv;
layout(location=1)out vec3 nrm;

void main()
{
	vec4 rotnrm;

	gl_Position = proj * view * mpos * mrot * vec4(vtxPos, 1.0);

	uv = vtxTex;
	
	rotnrm = mrot * vec4(vtxNrm, 1.0);
	nrm = rotnrm.xyz;
}
