#version 330 core

in vec3 vtxPos;
in vec2 vtxTex;
in vec3 vtxNrm;
in ivec4 vtxJnt;
in vec4 vtxWgt;

uniform mat4 mpos;
uniform mat4 mrot;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 jnts[100];

out vec2 uv;
out vec3 nrm;

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

	uv = vtxTex;
	nrm = (mrot * totalNrm).xyz;
}
