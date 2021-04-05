#version 330 core

out float fragCoord;

in vec2 v_texCoords;

uniform sampler2D u_gPosition;
uniform sampler2D u_gNormal;
uniform sampler2D u_texNoise;

uniform vec3 samples[64];
uniform mat4 u_projection; // camera projection matrix
uniform mat4 u_view; // camera view matrix

const vec2 noiseScale = vec2(840.0/4.0, 640.0/4.0); 

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

void main()
{
	
	vec3 fragPos   = vec3(u_view * vec4(texture(u_gPosition, v_texCoords).xyz,1));
	vec3 normal    = vec3(transpose(inverse(mat3(u_view))) * texture(u_gNormal, v_texCoords).rgb);
	vec3 randomVec = texture(u_texNoise, v_texCoords * noiseScale).xyz; 

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal); 


	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 samplePos = TBN * samples[i]; // from tangent to view-space
		//vec3 samplePos = TBN * normalize(vec3(0.5, 0.3, 0.4)); 
		samplePos = fragPos + samplePos * radius; 
		
		vec4 offset = vec4(samplePos, 1.0);
		offset = u_projection * offset; // from view to clip-space
		offset.xyz /= offset.w; // perspective divide
		offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
		
		// get sample depth
		float sampleDepth = vec3( u_view * vec4(texture(u_gPosition, offset.xy).xyz,1) ).z;
		//float sampleDepth = texture(u_gPosition, offset.xy).z; // get depth value of kernel sample
		
		// range check & accumulate
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;

	}  

	occlusion = 1.0 - (occlusion / kernelSize);

	fragCoord = occlusion;

	//fragCoord = normal.z;


}