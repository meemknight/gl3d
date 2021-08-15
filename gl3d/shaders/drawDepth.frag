#version 150 core

//layout(location = 0) out vec4 outColor;
out vec4 outColor;

in vec2 v_texCoords;


uniform sampler2D u_depth;

void main()
{
	float c = texture(u_depth, v_texCoords).r;
	outColor.rgba = vec4(c,c,c,1);
}