#version 330 core

out float fragColor;

noperspective in highp vec2 v_texCoords;

uniform sampler2D u_gPosition;
uniform isampler2D u_gNormal;
uniform sampler2D u_texNoise;

uniform mat4 u_projection; // camera projection matrix
uniform mat4 u_view; // camera view matrix

const float INFINITY = 9999999999999999999999999999999999.f*9999999999999999999999999999999999.f;

vec3 fromuShortToFloat(ivec3 a)
{
	vec3 ret = a;

	//[0 65536] -> [0 1]
	ret /= 65536;

	//[0 1] -> [0 2]
	ret *= 2.f;

	//[0 2] -> [-1 1]
	ret -= 1.f;

	return normalize(ret);
}


void main()
{
	vec2 screenSize = textureSize(u_gPosition, 0).xy/2.f; //smaller rez
	vec2 noiseScale = vec2(screenSize.x/4.0, screenSize.y/4.0);
	vec2 noisePos = v_texCoords * noiseScale;

	vec3 fragPos   = texture(u_gPosition, v_texCoords).xyz;
	vec3 normal    = normalize( vec3(transpose(inverse(mat3(u_view))) * 
		fromuShortToFloat(texture(u_gNormal, v_texCoords).xyz)));
	vec3 randomVec = texture2D(u_texNoise, noisePos).xyz; 

	//vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	//vec3 bitangent = cross(normal, tangent);
	//mat3 TBN       = mat3(tangent, bitangent, normal); 

	//vec3 tangent = normalize(normal);

	vec2 direction = normalize(vec2(0.5,0.5));
	vec3 leftDirection = cross(normal, vec3(direction, 0));
	vec3 tangent = normalize( cross(leftDirection, normal) );

	float tangentAngle = atan(tangent.z / length(tangent.xy));
	float sinTangentAngle = sin(tangentAngle);
	
	//

	vec2 texelSize = vec2(1.f,1.f) / screenSize;

	float highestZ = -INFINITY;
	vec3 foundPos = vec3(0,0,-INFINITY);
	for(int i=1; i<=10; i++)
	{
		vec2 marchPosition = v_texCoords + i*texelSize*direction;
		
		vec3 fragPosMarch = texture(u_gPosition, v_texCoords).xyz;
		
		if(fragPosMarch.z > highestZ)
		{
			highestZ = fragPosMarch.z;
			foundPos = fragPosMarch;
		}
	}

	vec3 horizonVector = foundPos - fragPos;
	
	float horizonAngle = atan(horizonVector.z/length(horizonVector.xy));
	float sinHorizonAngle = sin(horizonAngle);	


	fragColor = sinHorizonAngle - sinTangentAngle;

}