#version 330
#pragma debug(on)

out vec4 a_outColor;

in vec3 v_texCoords;

uniform samplerCube u_skybox;

uniform float u_gama;

vec3 toGama(in vec3 v)
{
	return v.rgb = pow(v.rgb, vec3(1.0/u_gama));
}

vec3 toLinear(in vec3 v)
{
	return v.rgb = pow(v.rgb, vec3(u_gama));
}

void main()
{    

	a_outColor = textureCube(u_skybox, v_texCoords);


	//a_outColor.rgb = toGama(a_outColor.rgb);
}