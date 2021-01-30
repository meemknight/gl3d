#version 330 
#pragma debug(on)

out vec4 a_outColor;

const vec3 color = vec3(0.7, 0.7, 0.1);

void main()
{
	a_outColor = vec4(color, 1);
}

