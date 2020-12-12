#version 330

in layout(location = 0) vec2 positions;

void main()
{

	gl_Position = vec4(positions,0,1);

} 