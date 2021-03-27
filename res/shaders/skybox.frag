#version 420

layout(location=0) in vec3 uvw;

layout(binding=1) uniform samplerCube skybox;

layout(location=0) out vec4 FragColor;

void main()
{
	FragColor = texture(skybox, uvw);
}