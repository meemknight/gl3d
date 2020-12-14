#version 330

in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;

uniform mat4 u_transform;
uniform mat4 u_modelTransform;

out vec3 v_normals;
out vec3 v_position;

void main()
{

	gl_Position = u_transform * vec4(a_positions, 1.f);

	v_position = (u_modelTransform * vec4(a_positions,1)).xyz;
	//v_normals = (u_modelTransform * vec4(a_normals,0)).xyz; //uniform scale
	v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * a_normals;  //non uniform scale

	v_normals = normalize(v_normals);

}