#version 330
#pragma debug(on)

layout(location = 0) in vec3 a_positions;
//layout(location = 1) in vec3 a_normals;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_transform; //full model view projection

out vec2 v_texCoord;

void main()
{
	
	gl_Position = u_transform * vec4(a_positions, 1.f);
	v_texCoord = a_texCoord;
	
}