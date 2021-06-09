#version 330 core

layout (location = 0) out vec3 a_color;

in vec2 v_texCoords;

uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;

uniform float u_bloomIntensity;

void main()
{
	vec3 color = texture(u_colorTexture, v_texCoords).rgb;
	vec3 bloom = texture(u_bloomTexture, v_texCoords).rgb;


	a_color = (bloom * u_bloomIntensity) + color;
	//a_color = bloom;
	
	//hdr
	float exposure = 1;
	color = vec3(1.0) - exp(-color  * exposure);

	//gamma correction
	a_color = pow(a_color, vec3(1.0/2.2));

}