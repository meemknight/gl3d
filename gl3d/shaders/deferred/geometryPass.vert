#version 330
#pragma debug(on)

layout(location = 0) in vec3 a_positions;
layout(location = 1) in vec3 a_normals;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_transform; //full model view projection
uniform mat4 u_modelTransform; //just model view

out vec3 v_normals;
out vec3 v_position;
out vec2 v_texCoord;

void main()
{

	gl_Position = u_transform * vec4(a_positions, 1.f);

	v_position = (u_modelTransform * vec4(a_positions,1)).xyz;

	//v_normals = (u_modelTransform * vec4(a_normals,0)).xyz; //uniform scale
	v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale

	v_normals = normalize(v_normals);

	v_texCoord = a_texCoord;
	
}