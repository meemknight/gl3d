#version 330
#pragma debug(on)

in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;

uniform mat4 u_modelTransform; //just model view

out vec3 v_normals;

void main()
{
	
	gl_Position = u_modelTransform * vec4(a_positions, 1);

	//v_normals = (u_modelTransform * vec4(a_normals,0)).xyz; //uniform scale
	v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale
	v_normals = normalize(v_normals);


}