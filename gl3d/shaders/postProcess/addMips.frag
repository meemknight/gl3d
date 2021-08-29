#version 150 core

out vec3 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;

void main()
{
	a_color = textureLod(u_texture, v_texCoords, u_mip).rgb;
}