#version 330
#pragma debug(on)

out layout(location = 0) vec4 outColor;

in vec3 v_colors;

void main()
{

	outColor = vec4(v_colors,1.0);
	

} 