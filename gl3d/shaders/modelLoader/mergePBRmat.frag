#version 430 core

in vec2 v_texCoords;
out vec4 fragColor;

layout(binding = 0) uniform sampler2D u_roughness;
layout(binding = 1) uniform sampler2D u_metallic;
layout(binding = 2) uniform sampler2D u_ambient;

void main()
{
	float metallic = texture(u_metallic, v_texCoords).r;
	float roughness = texture(u_roughness, v_texCoords).r;
	float ambient = texture(u_ambient, v_texCoords).r;

	fragColor = vec4(roughness, metallic, ambient, 1);

}