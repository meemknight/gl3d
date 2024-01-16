#version 150 core

//layout (location = 0) out vec4 a_color;
out vec4 a_color;

noperspective in vec2 v_texCoords;

uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;
uniform sampler2D u_bloomNotBluredTexture;

uniform float u_bloomIntensity;
uniform float u_exposure;

uniform int u_useSSAO;
uniform float u_ssaoExponent;
uniform sampler2D u_ssao;

//https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
/*
=================================================================================================

  Baking Lab
  by MJP and David Neubelt
  http://mynameismjp.wordpress.com/

  All code licensed under the MIT license

=================================================================================================
 The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
 credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)
*/

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3x3 ACESOutputMat = mat3x3
(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	// Apply RRT and ODT
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return color;
}

vec3 ACESFilmSimple(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);

}

void main()
{
	vec4 color = texture(u_colorTexture, v_texCoords).rgba;

	//a_color = color.aaa;
	//return;

	float ssaof = 1;
	if(u_useSSAO != 0)
	{
		ssaof = texture(u_ssao, v_texCoords).r;	
		ssaof = pow(ssaof, u_ssaoExponent);
	}else
	{
		ssaof = 1;
	}
	
	vec3 bloom = texture(u_bloomTexture, v_texCoords).rgb;
	vec3 bloomNotBlurred = texture(u_bloomNotBluredTexture, v_texCoords).rgb;


	//if(color.a < 0.5 && bloom.r == 0 && bloom.g == 0 && bloom.b == 0){discard;} //this is optional
	//if(color.a < 0.5){discard;}


	//a_color.rgb = (bloom * u_bloomIntensity) + (bloomNotBlurred + color.rgb) * ssaof;
	a_color.rgb = (bloom * u_bloomIntensity) + (color.rgb) * ssaof;
	//a_color = bloom;
	
	a_color.rgb = ACESFitted(a_color.rgb * u_exposure);
	//a_color.rgb = ACESFilmSimple(a_color.rgb * u_exposure);


	//hdr
	//float exposure = u_exposure;
	//a_color.rgb = vec3(1.0) - exp(-a_color.rgb  * exposure);

	//gamma correction
	a_color.rgb = pow(a_color.rgb, vec3(1.0/2.2));

	//a_color.rgb = vec3(ssaof);

	a_color.a = color.a;
}