#version 330 core

noperspective in vec2 v_texCoords;

uniform sampler2D u_depth;

void main()
{
	gl_FragDepth = texture(u_depth, v_texCoords).r;
}