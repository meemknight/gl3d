#version 150 core
/*
	http://advances.realtimerendering.com/s2014/index.html
	NEXT GENERATION POST PROCESSING IN CALL OF DUTY: ADVANCED WARFARE
*/

out vec3 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;


void main()
{
	vec2 texel = 1.f / textureSize(u_texture, u_mip).xy;
	a_color = vec3(0.f);

	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(-1, +1), u_mip).rgb;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(0, +1), u_mip).rgb*2;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(+1, +1), u_mip).rgb;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(-1, 0), u_mip).rgb*2;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(0, 0), u_mip).rgb*4;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(+1, 0), u_mip).rgb*2;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(-1, -1), u_mip).rgb;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(0, -1), u_mip).rgb*2;
	a_color	+= textureLod(u_texture, v_texCoords + texel*vec2(+1, -1), u_mip).rgb;

	a_color /= 16.f;


}

