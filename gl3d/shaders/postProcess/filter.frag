#version 150 core

out vec4 a_outBloom;
in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform float u_exposure;
uniform float u_tresshold;

void main()
{
	vec3 color = texture2D(u_texture, v_texCoords).rgb;

	vec3 hdrCorrectedColor = color.rgb;
	hdrCorrectedColor.rgb = vec3(1.0) - exp(-hdrCorrectedColor.rgb  * u_exposure);
	hdrCorrectedColor.rgb = pow(hdrCorrectedColor.rgb, vec3(1.0/2.2));
	float lightIntensity = dot(hdrCorrectedColor.rgb, vec3(0.2126, 0.7152, 0.0722));	

	if(lightIntensity > u_tresshold)
		{
			//a_outBloom = clamp(vec4(color.rgb, 1), 0, 1) + vec4(emissive.rgb, 0);
			//a_outColor = clamp(vec4(color.rgb, 1), 0, 1);	
	
			a_outBloom = vec4(color.rgb, 1);
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