#version 330
#pragma debug(on)


out vec4 a_outColor;

in vec3 v_texCoords;

uniform samplerCube u_skybox;



void main()
{    

	a_outColor = textureLod(u_skybox, v_texCoords, 2);

}