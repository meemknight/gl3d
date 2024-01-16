#version 150 core

out vec4 a_color;

noperspective in vec2 v_texCoords;

uniform sampler2D u_finalColorTexture;
uniform sampler2D u_DepthTexture;
uniform ivec2 u_windowSize;
uniform float u_strength;

uniform float u_near;
uniform float u_far;
uniform float u_unfocusDistance;


float toDistance(vec2 coords)
{
	float depth = texture(u_DepthTexture, v_texCoords).x;
	float ndcDepth = depth * 2.0 - 1.0; 
	float linearDepth = (2.0 * u_near * u_far) / (u_far + u_near - ndcDepth * (u_far - u_near));
	return linearDepth;
}

void main()
{

	
	vec2 strength = u_strength / u_windowSize;

	vec2 fragC = gl_FragCoord.xy;
	fragC /= u_windowSize; // 0, 1
	fragC *= 2; // 0, 2
	fragC -= 1; // -1, 1

	vec2 displacement = -fragC;

	vec2 displacementR = displacement*strength;
	vec2 displacementG = displacement*strength*0.5;

	float linearDepthR = toDistance(v_texCoords + displacementR);
	float linearDepthG = toDistance(v_texCoords + displacementG);
	float linearDepthCurrent = toDistance(v_texCoords);
	
	vec3 finalColor;

	if(linearDepthR < u_unfocusDistance && linearDepthCurrent < linearDepthR)
	{
		finalColor.r = texture(u_finalColorTexture, v_texCoords).r;
	}else
	{
		finalColor.r = texture(u_finalColorTexture, v_texCoords + displacementR).r;
	}

	if(linearDepthG < u_unfocusDistance && linearDepthCurrent < linearDepthG)
	{
		finalColor.g = texture(u_finalColorTexture, v_texCoords).g;
	}else
	{
		finalColor.g = texture(u_finalColorTexture, v_texCoords + displacementG).g;
	}

	finalColor.b = texture(u_finalColorTexture, v_texCoords).b;

	a_color = vec4(finalColor, 1);


}