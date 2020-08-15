#version 330 core

in vec2 uv;
flat in vec3 nrm;

vec3 LIGHT_VEC = -vec3(0.259, -0.864, 0.432);
vec3 LIGHT_COL = vec3(1.0, 1.0, 1.0);
vec2 LIGHT_BIAS = vec2(0.2, 0.8);

uniform sampler2D tex;

out vec4 FragColor;

vec3 calculateLighting(){
	float brightness = clamp((dot(nrm, LIGHT_VEC) + 1) / 2, 0.0, 1.0);
	return (LIGHT_COL * LIGHT_BIAS.x) + (brightness * LIGHT_COL * LIGHT_BIAS.y);
}

void main()
{
	vec3 dark = calculateLighting();
	FragColor = vec4(texture(tex, uv).rgb * dark, 1.0);
} 
