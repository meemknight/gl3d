#version 330 core

layout (location = 0) out vec3 a_color;

in vec2 v_texCoord;

uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;



void main()
{
	vec3 color = texture(u_colorTexture, v_texCoord).rgb;
	vec3 bloom = texture(u_bloomTexture, v_texCoord).rgb;

	a_color = bloom + color;

}