#version 330 core

layout (location = 0) out vec4 a_color;

in vec2 v_texCoords;

uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;

uniform float u_bloomIntensity;
uniform float u_exposure;

void main()
{
	vec4 color = texture(u_colorTexture, v_texCoords).rgba;

	//a_color = color.aaa;
	//return;

	
	vec3 bloom = texture(u_bloomTexture, v_texCoords).rgb;

	if(color.a < 0.5 && bloom.r == 0 && bloom.g == 0 && bloom.b == 0){discard;} //this is optional
	//if(color.a < 0.5){discard;}


	a_color.rgb = (bloom * u_bloomIntensity) + color.rgb;
	//a_color = bloom;
	
	//hdr
	float exposure = u_exposure;
	a_color.rgb = vec3(1.0) - exp(-a_color.rgb  * exposure);

	//gamma correction
	a_color.rgb = pow(a_color.rgb, vec3(1.0/2.2));

	a_color.a = color.a;
}