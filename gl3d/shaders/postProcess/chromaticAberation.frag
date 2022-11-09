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
	

	float linearDepth = toDistance(v_texCoords);
	//float depth = texture(u_DepthTexture, v_texCoords).x;
	//a_color = vec4(linearDepth ,linearDepth ,linearDepth , 1);
	//return;
	
	if(linearDepth < u_unfocusDistance)
	{
		a_color = vec4(texture(u_finalColorTexture, v_texCoords).rgb, 1);
		
	}else
	{
		vec2 strength = u_strength / u_windowSize;

		vec2 fragC = gl_FragCoord.xy;
		fragC /= u_windowSize; // 0, 1
		fragC *= 2; // 0, 2
		fragC -= 1; // -1, 1

		vec2 displacement = -fragC;

		float r = texture(u_finalColorTexture, v_texCoords + displacement*strength).r;
		float g = texture(u_finalColorTexture, v_texCoords + displacement*strength*0.5).g;
		float b = texture(u_finalColorTexture, v_texCoords).b;


		a_color = vec4(r,g,b,1);

	}


}