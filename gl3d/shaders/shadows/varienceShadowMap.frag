#version 330
#pragma debug(on)

uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;

out vec2 outColor;

void main()
{
	if(u_hasTexture != 0)
	{
		float color = texture2D(u_albedoSampler, v_texCoord).w;
			if(color <= 0.1)
				discard;
	}

	outColor = vec2(gl_FragCoord.z, gl_FragCoord.z * gl_FragCoord.z);

}