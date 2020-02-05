#version 330 core

// inputs
in vec2 uv;
in vec3 dark;

// uniforms
uniform sampler2D tex;

// outputs
out vec3 color;

void main()
{
	color = texture(tex, uv).rgb * dark;
} 
