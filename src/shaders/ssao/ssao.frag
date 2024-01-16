#version 330 core

out float fragColor;

noperspective in highp vec2 v_texCoords;

uniform sampler2D u_gPosition; //position view space
uniform isampler2D u_gNormal;
uniform sampler2D u_texNoise;

uniform vec3 samples[64];
uniform mat4 u_projection; // camera projection matrix
uniform mat4 u_view; // camera view matrix


const int kernelSize = 64;

layout(std140) uniform u_SSAODATA
{
	float radius;
	float bias;
	int samplesTestSize; // should be less than kernelSize

}ssaoDATA;

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

	vec3 fragPos = texture(u_gPosition, v_texCoords).xyz;

	vec3 normal    = normalize( vec3(transpose(inverse(mat3(u_view))) * 
		fromuShortToFloat(texture(u_gNormal, v_texCoords).xyz)) );
	vec3 randomVec = texture2D(u_texNoise, noisePos).xyz; 

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 


	float occlusion = 0.0;


	int begin = int((kernelSize - ssaoDATA.samplesTestSize) * abs(randomVec.x));

	for(int i = begin; i < begin + ssaoDATA.samplesTestSize; ++i)
	{
		vec3 samplePos = TBN * samples[i]; // from tangent to view-space
		//vec3 samplePos = TBN * normalize(vec3(0.5, 0.3, 0.4)); 
		samplePos = fragPos + samplePos * ssaoDATA.radius; 
		
		vec4 offset = vec4(samplePos, 1.0);
		offset = u_projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
		//if(dot(normal, normalize(offset.xyz)) > 0.02) //1.14 degrees
		{
			// get sample depth
			//float sampleDepth = vec3( u_view * vec4(texture(u_gPosition, offset.xy).xyz,1) ).z;
			float sampleDepth = texture(u_gPosition, offset.xy).z; // get depth value of kernel sample
			
			// range check & accumulate
			float rangeCheck = smoothstep(0.0, 1.0, ssaoDATA.radius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= samplePos.z + ssaoDATA.bias ? 1.0 : 0.0) * rangeCheck;
		}

	}  

	occlusion = 1.0 - (occlusion / kernelSize);

	fragColor = occlusion;

	//fragColor = v_texCoords.y;
	//fragColor = normal.z;
	//fragColor = sqrt(abs(randomVec.x));

}