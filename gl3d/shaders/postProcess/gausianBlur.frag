#version 150 core

in vec2 v_texCoords;

uniform sampler2D u_toBlurcolorInput;

out vec3 fragColor;


uniform bool u_horizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
	vec2 texOffset = 1.0 / textureSize(u_toBlurcolorInput, 0); // gets size of single texel
	vec3 result = texture(u_toBlurcolorInput, v_texCoords).rgb * weight[0]; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(u_toBlurcolorInput, v_texCoords + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
			result += texture(u_toBlurcolorInput, v_texCoords - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(u_toBlurcolorInput, v_texCoords + vec2(0.0, texOffset.y * i)).rgb * weight[i];
			result += texture(u_toBlurcolorInput, v_texCoords - vec2(0.0, texOffset.y * i)).rgb * weight[i];
		}
	}

	fragColor = vec3(result);
	
}