#version 330

in layout(location = 0) vec3 positions;

uniform mat4 u_transform;

void main()
{

	gl_Position = u_transform * vec4(positions,1.f);

} 