#version 150
#pragma debug(on)

uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;


void main()
{
	if(u_hasTexture != 0)
	{
		vec4 color = texture2D(u_albedoSampler, v_texCoord).xyzw;
			if(color.w <= 0.1)
				discard;
	}
}