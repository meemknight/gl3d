#version 150 core
out vec4 a_color;

noperspective in vec2 v_texCoords;

uniform sampler2D u_texture;

//used when doing adaptive rezolution

void main()
{
	vec3 color = texture2D(u_texture, v_texCoords).rgb;
	a_color = vec4(color, 1);
}