#version 150
#pragma debug(on)

uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;


void main()
{
	if(u_hasTexture != 0)
	{
		float alphaData = texture2D(u_albedoSampler, v_texCoord).a;
			if(alphaData*255 < 1)
				discard;
	}
}