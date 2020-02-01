#version 330 core

in vec3 vtxPos;
in vec3 vtxNrm;
in vec2 vtxUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 uv;
out vec3 dark;

vec3 LIGHT_VEC = -vec3(0.259, -0.864, 0.432);
vec3 LIGHT_COL = vec3(1.0, 0.8, 0.8);
vec2 LIGHT_BIAS = vec2(0.2, 0.8);

vec3 calculateLighting(){
	normalize(vtxNrm);
	float brightness = clamp((dot(vtxNrm, LIGHT_VEC) + 1) / 2, 0.0, 1.0);
	return (LIGHT_COL * LIGHT_BIAS.x) + (brightness * LIGHT_COL * LIGHT_BIAS.y);
}

void main()
{
	gl_Position = proj * view * model * vec4(vtxPos, 1.0);

	uv = vtxUV;
	dark = calculateLighting();
}
