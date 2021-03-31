#version 420

layout(location=0) in vec3 vtxPos;

layout(binding=0) uniform UBO {
	mat4 mpos;
	mat4 mrot;
	mat4 view;
	mat4 proj;
};

layout(location=0) out vec3 uvw;

void main()
{
	uvw = vtxPos;

	mat4 tmp = view;
	tmp[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	gl_Position = proj * tmp * vec4(vtxPos, 1.0);
}