#version 330 core

in vec2 v_texCoords;
uniform sampler2D u_ssaoInput;

out float fragColor;


void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(u_ssaoInput, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x) 
	{
		for (int y = -2; y < 2; ++y) 
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(u_ssaoInput, v_texCoords + offset).r;
		}
	}

	fragColor = result / (4.0 * 4.0);

	//fragColor = texture(u_ssaoInput, v_texCoords).r;
}