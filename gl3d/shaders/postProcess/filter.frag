#version 150 core

out vec4 a_outBloom;
in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float u_exposure;
uniform float u_tresshold;

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


void main()
{
	vec3 color = texture2D(u_texture, v_texCoords).rgb;

	vec3 hdrCorrectedColor = color.rgb;
	hdrCorrectedColor = ACESFitted(hdrCorrectedColor * u_exposure);
	//hdrCorrectedColor.rgb = vec3(1.0) - exp(-hdrCorrectedColor.rgb  * u_exposure);
	//hdrCorrectedColor.rgb = pow(hdrCorrectedColor.rgb, vec3(1.0/2.2));
	float lightIntensity = dot(hdrCorrectedColor.rgb, vec3(0.2126, 0.7152, 0.0722));	

	if(lightIntensity > u_tresshold)
		{
			//a_outBloom = clamp(vec4(color.rgb, 1), 0, 1) + vec4(emissive.rgb, 0);
			//a_outColor = clamp(vec4(color.rgb, 1), 0, 1);	
	
			a_outBloom = vec4(hdrCorrectedColor.rgb, 1);
			//a_outColor = vec4(color.rgb, albedoAlpha.a);	
			//a_outColor = vec4(0,0,0, albedoAlpha.a);	
	
		}else
		{
			//a_outBloom = vec4(0, 0, 0, 0) + vec4(emissive.rgb, 0); //note (vlod) keep this here
			//a_outColor = clamp(vec4(color.rgb, 1), 0, 1);
	
			a_outBloom = vec4(0,0,0, 1);
			//a_outColor = vec4(color.rgb, albedoAlpha.a);
		}
	
		a_outBloom = clamp(a_outBloom, 0, 1000);

}