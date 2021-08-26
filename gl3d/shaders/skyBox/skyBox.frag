#version 150
#pragma debug(on)


out vec4 a_outColor;

in vec3 v_texCoords;

uniform samplerCube u_skybox;
uniform float u_exposure;
uniform vec3 u_ambient;
uniform int u_skyBoxPresent;

void main()
{    
	
	vec3 gammaAmbient = pow(u_ambient, vec3(2.2)).rgb; //gamma corection

	if(u_skyBoxPresent != 0)
	{
		a_outColor = textureLod(u_skybox, v_texCoords, 2);
		a_outColor.rgb *= gammaAmbient;
	}else
	{
		a_outColor.rgb = gammaAmbient;
	}

	//hdr
	float exposure = u_exposure;
	a_outColor.rgb = vec3(1.0) - exp(-a_outColor.rgb  * exposure);

	//gama
	a_outColor.rgb = pow(a_outColor.rgb, vec3(1.0/2.2)); 

}