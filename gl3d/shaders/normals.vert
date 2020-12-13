#version 330

in layout(location = 0) vec3 positions;
in layout(location = 1) vec3 normals;

uniform mat4 u_transform;

out vec3 v_normals;
out vec3 v_position;

void main()
{

	gl_Position = u_transform * vec4(positions, 1.f);

	v_position = positions;
	v_normals = normals;


}