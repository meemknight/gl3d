#version 330
#pragma debug(on)


out vec4 a_outColor;

in vec3 v_texCoords;

uniform samplerCube u_skybox;



void main()
{    

	a_outColor = textureLod(u_skybox, v_texCoords, 2);
	//hdr
	float exposure = 1;
	a_outColor.rgb = vec3(1.0) - exp(-a_outColor.rgb  * exposure);
	a_outColor.rgb = pow(a_outColor.rgb, vec3(1.0/2.2)); 

}