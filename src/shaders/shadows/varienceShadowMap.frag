#version 150
#pragma debug(on)

uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;

out vec2 outColor;

//https://www.youtube.com/watch?v=F5QAkUloGOs&ab_channel=thebennybox
void main()
{
	if(u_hasTexture != 0)
	{
		float color = texture2D(u_albedoSampler, v_texCoord).w;
			if(color <= 0.1)
				discard;
	}

	float zVal = gl_FragCoord.z;
	float zVal2 = zVal*zVal;

	float dx = dFdx(zVal);
	float dy = dFdy(zVal);

	float moment2 = zVal2 + 0.25 * (dx * dx + dy * dy);

	//outColor = vec2(zVal, moment2);
	outColor = vec2(zVal, zVal2);

}