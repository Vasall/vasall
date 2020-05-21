#version 330 core

in vec2 uv;
in vec3 dark;

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
	FragColor = vec4(texture(tex, uv).rgb * dark, 1.0);
} 
