#version 330
#pragma debug(on)

in layout(location = 0) vec3 positions;
in layout(location = 1) vec3 colors;

uniform mat4 u_transform;

out vec3 v_colors;

void main()
{

	gl_Position = u_transform * vec4(positions,1.f);
	v_colors = colors;

} 