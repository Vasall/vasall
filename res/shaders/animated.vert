#version 420

layout(location=0) in vec3  vtxPos;
layout(location=1) in vec2  vtxTex;
layout(location=2) in vec3  vtxNrm;
layout(location=3) in ivec4 vtxJnt;
layout(location=4) in vec4  vtxWgt;

layout(binding=0) uniform UBO {
	mat4 mpos;
	mat4 mrot;
	mat4 view;
	mat4 proj;
	mat4 jnts[100];
};

layout(location=0) out vec2 uv;
layout(location=1) out vec3 nrm;

void main()
{
	vec4 rotnrm;
	vec4 totalLocPos = vec4(0.0);
	vec4 totalNrm = vec4(0.0);

	for(int i = 0; i < 4; i++) {
		if(vtxJnt[i] < 0)
			continue;

		mat4 jntTrans = jnts[vtxJnt[i]];
		vec4 posePos = jntTrans * vec4(vtxPos, 1.0);
		totalLocPos += posePos * vtxWgt[i];

		vec4 wldNrm = jntTrans * vec4(vtxNrm, 0.0);
		totalNrm += wldNrm * vtxWgt[i];
	}

	gl_Position = proj * view * mpos * mrot * totalLocPos;
	gl_Position[0] *= -1;

	uv = vtxTex;
	nrm = (mrot * totalNrm).xyz;
}
