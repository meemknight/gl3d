#version 330

in layout(location = 0) vec3 a_positions;
in layout(location = 1) vec3 a_normals;

uniform mat4 u_transform;
uniform mat4 u_normalTransform;


out vec3 v_normals;
out vec3 v_position;

void main()
{

	gl_Position = u_transform * vec4(a_positions, 1.f);

	v_position = (u_normalTransform * vec4(a_positions,1)).xyz;
	//v_normals = (u_normalTransform * vec4(a_normals,0)).xyz;
	v_normals = mat3(transpose(inverse(u_normalTransform))) * a_normals;  

	v_normals = normalize(v_normals);

}