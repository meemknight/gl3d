#version 430 core
#pragma debug(on)

//#extension GL_NV_shadow_samplers_cube : enable
//#extension ARB_program_interface_query : enable
#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 a_outColor;
layout(location = 1) out vec4 a_outBloom;

noperspective in vec2 v_texCoords;

uniform isampler2D u_normals;
uniform samplerCube u_skyboxFiltered;
uniform samplerCube u_skyboxIradiance;
uniform sampler2D u_positions;
uniform sampler2D u_brdfTexture;
uniform sampler2DArrayShadow u_cascades;
uniform sampler2DArrayShadow u_spotShadows;
uniform samplerCubeArrayShadow u_pointShadows;
uniform isampler2D u_materialIndex;
uniform sampler2D u_textureUV;
uniform sampler2D u_lastFrameTexture;
uniform sampler2D u_positionViewSpace;
uniform isampler2D u_textureDerivates;
uniform int u_hasLastFrameTexture;
//uniform sampler2D u_textureDerivates;
uniform mat4 u_cameraProjection;


uniform vec3 u_eyePosition;
uniform int u_transparentPass;

struct MaterialStruct
{
	vec4 kd;
	vec4 rma; //last component emmisive
	
	//float kdr; //= 1;
	//float kdg; //= 1;
	//float kdb; //= 1;
	//float roughness;
	//float metallic;
	//float ao; //one means full light

	//layout(bindless_sampler) sampler2D albedoSampler;
	//layout(bindless_sampler) sampler2D rmaSampler;
	//layout(bindless_sampler) sampler2D emmissiveSampler;
	//vec2 notUsed;

	uvec4 firstBIndlessSamplers;  // xy albedoSampler,  zw rmaSampler
	uvec2 secondBIndlessSamplers; // xy emmissiveSampler
	int rmaLoaded;
	int notUsed;

};
readonly layout(std140) buffer u_material
{
	MaterialStruct mat[];
};


layout (std140) uniform u_lightPassData
{
	vec4 ambientColor;
	float bloomTresshold;
	int lightSubScater;
	float exposure;
	int skyBoxPresent;
}lightPassData;

struct PointLight
{
	vec3 positions; 
	float dist;
	vec3 color;
	float attenuation;
	int castShadowsIndex;
	float hardness;
	int castShadows;
	int changedThisFrame;
};
readonly restrict layout(std140) buffer u_pointLights
{
	PointLight light[];
};
uniform int u_pointLightCount;


struct DirectionalLight
{
	vec3 direction; 
	int castShadowsIndex;

	int changedThisFrame; //not used here
	int castShadows;
	int notUsed1;
	int notUsed2;

	vec4 color;		//w is a hardness exponent
	mat4 firstLightSpaceMatrix;
	mat4 secondLightSpaceMatrix;
	mat4 thirdLightSpaceMatrix;

};
readonly restrict layout(std140) buffer u_directionalLights
{
	DirectionalLight dLight[];
};
uniform int u_directionalLightCount;


struct SpotLight
{
	vec4 position; //w = cos(half angle)
	vec4 direction; //w dist
	vec4 color; //w attenuation
	float hardness;
	int shadowIndex;
	int castShadows;		
	int changedThisFrame; //not used in the gpu
	float near;
	float far;
	float notUsed1;
	float notUsed2;
	mat4 lightSpaceMatrix;
};
readonly restrict layout(std140) buffer u_spotLights
{
	SpotLight spotLights[];
};
uniform int u_spotLightCount;



const float PI = 3.14159265359;

const float randomNumbers[100] = float[100](
0.05535,	0.22262,	0.93768,	0.80063,	0.40089,	0.49459,	0.44997,	0.27060,	0.58789,	0.61765,
0.87949,	0.38913,	0.23154,	0.27249,	0.93448,	0.71567,	0.26940,	0.32226,	0.73918,	0.30905,
0.98754,	0.82585,	0.84031,	0.60059,	0.56027,	0.10819,	0.55848,	0.95612,	0.88034,	0.94950,
0.53892,	0.86421,	0.84131,	0.39158,	0.25861,	0.10192,	0.19673,	0.25165,	0.68675,	0.79157,
0.94730,	0.36948,	0.27978,	0.66377,	0.38935,	0.93795,	0.83168,	0.01452,	0.51242,	0.12272,
0.61045,	0.34752,	0.13781,	0.92361,	0.73422,	0.31213,	0.55513,	0.81074,	0.56166,	0.31797,
0.09507,	0.50049,	0.44248,	0.38244,	0.58468,	0.32327,	0.61830,	0.67908,	0.16011,	0.82861,
0.36502,	0.12052,	0.28872,	0.73448,	0.51443,	0.99355,	0.75244,	0.22432,	0.95501,	0.90914,
0.37992,	0.61330,	0.49202,	0.69464,	0.14831,	0.51697,	0.34620,	0.55315,	0.41602,	0.49807,
0.15133,	0.07372,	0.75259,	0.59642,	0.35652,	0.60051,	0.08879,	0.59271,	0.29388,	0.69505
);

const float INFINITY = 1.f/0.f;


float attenuationFunctionNotClamped(float x, float r, float p)
{

	float p4 = p*p*p*p;
	float power = pow(x/r, p4);

	float rez = (1-power);
	rez = rez * rez;
	
	return rez;

}


//n normal
//h halfway vector
//a roughness	(1 rough, 0 glossy) 
//this gets the amount of specular light reflected
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	//GGX/Trowbridge-Reitz
	//			 a^2
	// ------------------------
	// PI ((N*H)^2 (a^2-1)+1)^2

	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return  a2 / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	//float r = (roughness + 1.0);
	//float k = (r*r) / 8.0;			//disney

	float k = roughness*roughness / 2;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / max(denom, 0.0000001);
}
 
//oclude light that is hidded begind small geometry roughnesses
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}


//cosTheta is the dot between the normal and halfway
//ratio between specular and diffuse reflection
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

vec3 fSpecular(vec3 normal, vec3 halfwayVec, vec3 viewDir, 
vec3 lightDirection, float dotNVclamped, float roughness, vec3 F)
{
	//fCook-Torrance
	float NDF = DistributionGGX(normal, halfwayVec, roughness);       
	float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   
	float denominator = 4.0 * dotNVclamped  
		* max(dot(normal, lightDirection), 0.0);
	vec3 specular     = (NDF * G * F) / max(denominator, 0.001);

	return specular;
}

vec3 fDiffuse(vec3 color)
{
	//fLambert
	return color.rgb / PI;
}

vec3 fDiffuseOrenNayar(vec3 color, float roughness, vec3 L, vec3 V, vec3 N)
{
	float a = roughness;
	float a2 = a*a;

	float cosi = max(dot(L, N), 0);
	float cosr = max(dot(V, N), 0);
	float sini = sqrt(1-cosi*cosi);
	float sinr = sqrt(1-cosr*cosr);
	float tani = sini/cosi;
	float tanr = sinr/cosr;

	float A = 1 - 0.5 * a2/(a2 + 0.33);
	float B = 0.45*a2/(a2+0.09);
	
	float sinAlpha = max(sini, sinr);
	float tanBeta = min(tani, tanr);

	return color.rgb * (A + (B* max(0, dot(L,reflect(V,N))) * sinAlpha * tanBeta  )) / PI;
}

//https://mimosa-pudica.net/improved-oren-nayar.html

vec3 fDiffuseOrenNayar2(vec3 color, float roughness, vec3 L, vec3 V, vec3 N)
{
	float a = roughness;
	float a2 = a*a;
	//vec3 A = 1.f/PI * (1 - 0.5 * a2/(a2 + 0.33) + 0.17*color*a2/(a2+0.13));
	//float B = 0.45*a2/(a2+0.09);

	float A = 1.0/(PI+(PI/2.0-2/3.0)*a);
	float B = PI/(PI+(PI/2.0-2/3.0)*a);

	float s = dot(L,N) - dot(N,L)*dot(N,V);

	float t;
	if(s <= 0)
		t = 1;
	else
		t = max(dot(N,L), dot(N,V));

	return color * (A + B * s/t);
}


vec3 computePointLightSource(vec3 lightDirection, float metallic, float roughness, in vec3 lightColor, in vec3 worldPosition,
	in vec3 viewDir, in vec3 color, in vec3 normal, in vec3 F0)
{
	//lightDirection = direction to light
	//vec3 lightDirection = normalize(lightPosition - worldPosition);

	float dotNVclamped = clamp(dot(normal, viewDir), 0.0, 0.99);

	vec3 halfwayVec = normalize(lightDirection + viewDir);
	
	vec3 radiance = lightColor; //here the first component is the light color
	
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	vec3 specular = fSpecular(normal, halfwayVec, viewDir, lightDirection, dotNVclamped, roughness, F);

	vec3 kS = F; //this is the specular contribution
	vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
	kD *= 1.0 - metallic;	//metallic surfaces are darker
	
	//vec3 diffuse = fDiffuse(color.rgb);
	//vec3 diffuse = fDiffuseOrenNayar(color.rgb, roughness, lightDirection, viewDir, normal);
	vec3 diffuse = fDiffuseOrenNayar2(color.rgb, roughness, lightDirection, viewDir, normal);
	
	float NdotL = max(dot(normal, lightDirection), 0.0);        
	return (kD * diffuse + specular) * radiance * NdotL;

}

float testShadowValue(sampler2DArrayShadow map, vec2 coords, float currentDepth, float bias, int index)
{
	//float closestDepth = texture(map, coords).r; 
	//return  (currentDepth - bias) < closestDepth  ? 1.0 : 0.0;

	return texture(map, vec4(coords, index, currentDepth-bias)).r;

}

//https://www.youtube.com/watch?v=yn5UJzMqxj0&ab_channel=thebennybox
//float sampleShadowLinear(sampler2DArrayShadow map, vec2 coords, vec2 texelSize, float currentDepth, float bias)
//{
//
//	vec2 pixelPos = coords / texelSize + vec2(0.5);
//	vec2 fracPart = fract(pixelPos);
//	vec2 startTexel = (pixelPos-fracPart) * texelSize;
//
//	float blTexture = testShadowValue(map, startTexel, currentDepth, bias).r;
//	float brTexture = testShadowValue(map, startTexel + vec2(texelSize.x, 0), currentDepth, bias).r;
//	float tlTexture = testShadowValue(map, startTexel + vec2(0, texelSize.y), currentDepth, bias).r;
//	float trTexture = testShadowValue(map, startTexel + texelSize, currentDepth, bias).r;
//
//	float mixA = mix(blTexture, tlTexture, fracPart.y);
//	float mixB = mix(brTexture, trTexture, fracPart.y);
//
//	return mix(mixA, mixB, fracPart.x);
//}

//https://developer.download.nvidia.com/cg/sincos.html
void sincos(float a, out float s, out float c)
{
	s = sin(a);
	c = cos(a);
}

//https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
vec2 vogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
  float GoldenAngle = 2.4f;

  float r = sqrt(sampleIndex + 0.5f) / sqrt(samplesCount);
  float theta = sampleIndex * GoldenAngle + phi;

  float sine, cosine;
  sincos(theta, sine, cosine);
  
  return vec2(r * cosine, r * sine);
}

//https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
float InterleavedGradientNoise(vec2 position_screen)
{
  vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

float shadowCalculation(vec3 projCoords, float bias, sampler2DArrayShadow shadowMap, int index)
{

	// keep the shadow at 1.0 when outside or close to the far_plane region of the light's frustum.
	if(projCoords.z > 0.99995)
		return 1.f;
	//if(projCoords.z < 0)
	//	return 1.f;

	//float closestDepth = texture(shadowMap, projCoords.xy).r; 
	float currentDepth = projCoords.z;

	//todo move
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
	float shadow = 0.0;

	bool fewSamples = false;
	int kernelHalf = 1;
	int kernelSize = kernelHalf*2 + 1;
	int kernelSize2 = kernelSize*kernelSize;

	//float receiverDepth = currentDepth;
	//float averageBlockerDepth = texture(sampler2DArray(shadowMap), vec3(projCoords.xy, index)).r;
	//float penumbraSize = 4.f * (receiverDepth - averageBlockerDepth) / averageBlockerDepth;
	float penumbraSize = 1.f;

	//standard implementation
	if(false)
	{
		float shadowValueAtCentre = 0;

		//optimization
		if(false)
		{
			float offsetSize = kernelSize/2;
			const int OFFSETS = 4;
			vec2 offsets[OFFSETS] = 
			{
				vec2(offsetSize,offsetSize),
				vec2(-offsetSize,offsetSize),
				vec2(offsetSize,-offsetSize),
				vec2(-offsetSize,-offsetSize),
			};

			fewSamples = true;
			
			float s1 = testShadowValue(shadowMap, projCoords.xy, 
						currentDepth, bias, index); 
			shadowValueAtCentre = s1;

			for(int i=0;i<OFFSETS; i++)
			{
				float s2 = testShadowValue(shadowMap, projCoords.xy + offsets[i] * texelSize * 2, 
						currentDepth, bias, index); 
				if(s1 != s2)
				{
					fewSamples = false;
					break;
				}	
				s1 = s2;
			}
		}


		if(fewSamples)
		{
			
			shadow = shadowValueAtCentre;

		}else
		{

			for(int y = -kernelHalf; y <= kernelHalf; ++y)
			{
				for(int x = -kernelHalf; x <= kernelHalf; ++x)
				{
					vec2 offset = vec2(x, y);
			
					if(false)
					{
						int randomOffset1 = (x*kernelSize) + y;
						int randomOffset2 = randomOffset1 + kernelSize2;
						offset += vec2(randomNumbers[randomOffset1, randomOffset2]);
					}
					
					if(false)
					{
						float u = (offset.x + kernelHalf)/float(kernelSize-1);
						float v = (offset.y + kernelHalf)/float(kernelSize-1);
						offset.x = sqrt(v) * cos(2*PI * u)* kernelHalf;
						offset.y = sqrt(v) * sin(2*PI * u)* kernelHalf;
					}
			
					vec2 finalOffset = offset * texelSize * penumbraSize;
					//float newDepth = sqrt(currentDepth*currentDepth + finalOffset.x*finalOffset.x + finalOffset.y * finalOffset.y);
			
					float s = testShadowValue(shadowMap, projCoords.xy + finalOffset, 
						currentDepth, bias, index); 
					
					//float s = sampleShadowLinear(shadowMap, projCoords.xy + vec2(x, y) * offset,
					//	texelSize, currentDepth, bias); 
					
					shadow += s;
				}    
			}
			shadow /= kernelSize2;


			// texture(map, vec4(coords, index, currentDepth-bias)).r;

			//for(int y = -kernelHalf; y <= kernelHalf; ++y)
			//{
			//	for(int x = -kernelHalf; x <= kernelHalf; ++x)
			//	{
			//		vec2 offset = vec2(x, y);
			//		vec2 finalOffset = offset * texelSize * 2;
			//
			//		vec4 shadowGather = 
			//			textureGather(shadowMap, vec3(projCoords.xy+finalOffset,
			//				index), currentDepth-bias); 
			//
			//		shadow += shadowGather.r;
			//		shadow += shadowGather.g;
			//		shadow += shadowGather.b;
			//		shadow += shadowGather.a;
			//
			//	}
			//}
			//
			//shadow /= 4*kernelSize2;
			

		}
	}else
	{
	
		int sampleSize = 9;
		int checkSampleSize = 5;
		float size = 1.5;

		float noise = InterleavedGradientNoise(v_texCoords) * 2 * PI;

		for(int i=sampleSize-1; i>=sampleSize-checkSampleSize; i--)
		{
			vec2 offset = vogelDiskSample(i, sampleSize, noise);
			vec2 finalOffset = offset * texelSize * size;
			
			float s = testShadowValue(shadowMap, projCoords.xy + finalOffset, 
				currentDepth, bias, index);
			shadow += s;
		}

		//optimization
		if(true && (shadow == 0 || shadow == checkSampleSize))
		{
			shadow /= checkSampleSize;
		}else
		{
			for(int i=sampleSize-checkSampleSize-1; i>=0; i--)
			{
				vec2 offset = vogelDiskSample(i, sampleSize, noise);
				vec2 finalOffset = offset * texelSize * size;
				
				float s = testShadowValue(shadowMap, projCoords.xy + finalOffset, 
					currentDepth, bias, index);
				shadow += s;
			}

			shadow /= sampleSize;
		}

	}
	
	return clamp(shadow, 0, 1);
}

float shadowCalculationLinear(vec3 projCoords, vec3 normal, vec3 lightDir, sampler2DArrayShadow shadowMap, int index)
{
	float bias = max((10.f/1024.f) * (1.0 - dot(normal, -lightDir)), 3.f/1024.f);
	return shadowCalculation(projCoords, bias, shadowMap, index);
}

float linearizeDepth(float depth, float near, float far)
{
	float z = depth * 2.0 - 1.0; // Back to NDC 
	return (2.0 * near * far) / (far + near - z * (far - near));
}

float nonLinearDepth(float depth, float near, float far)
{
	return ((1.f/depth) - (1.f/near)) / ((1.f/far) - (1.f/near));

}

//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-17-efficient-soft-edged-shadows-using
float shadowCalculationLogaritmic(vec3 projCoords, vec3 normal, vec3 lightDir,
sampler2DArrayShadow shadowMap, int index, float near, float far)
{
	//float bias = max((0.00005) * (1.0 - dot(normal, -lightDir)), 0.00001);
	float bias = max((0.01f) * (1.0 - dot(normal, -lightDir)), 0.001f);
	
	//bias = nonLinearDepth(bias, near, far);
	float currentDepth = projCoords.z;
	float liniarizedDepth = linearizeDepth(currentDepth, near, far);
	liniarizedDepth += bias;
	float biasedLogDepth = nonLinearDepth(liniarizedDepth, near, far);

	bias = biasedLogDepth - currentDepth;
	bias += 0.00003f;

	return shadowCalculation(projCoords, bias, shadowMap, index);
}

vec3 getProjCoords(in mat4 matrix, in vec3 pos)
{
	vec4 p = matrix * vec4(pos,1);
	vec3 r = p .xyz / p .w;
	r = r * 0.5 + 0.5;
	return r;
}

void generateTangentSpace(in vec3 v, out vec3 outUp, out vec3 outRight)
{
	vec3 up = vec3(0.f, 1.f, 0.f);

	if (v == up)
	{
		outRight = vec3(1, 0, 0);
	}
	else
	{
		outRight = normalize(cross(v, up));
	}

	outUp = normalize(cross(outRight, v));

}

float pointShadowCalculation(vec3 pos, vec3 normal, int index)
{	
	vec3 fragToLight = pos - light[index].positions; 
	vec3 lightDir = normalize(fragToLight);

	//float closestDepth = texture(u_pointShadows, lightDir).r;
	//closestDepth *= light[index].dist; //multiply by far plane

	
	float bias = max((60.f/512.f) * (1.0 - dot(normal, -lightDir)), 35.f/512.f);

	//float shadow = currentDepth -  bias < closestDepth ? 1.0 : 0.0; 
	float shadow  = 0.0;

	vec3 tangent;
	vec3 coTangent;

	generateTangentSpace(lightDir, tangent, coTangent);
	float texel = 1.f / textureSize(u_pointShadows, 0).x;

	//todo fix for even numbers
	int kernel = 5;
	int kernelHalf = kernel/2;

	for(int y = -kernelHalf; y<=kernelHalf; y++)
	{
		for(int x = -kernelHalf; x<=kernelHalf; x++)
		{
			vec3 fragToLight = pos - light[index].positions; 			
			fragToLight += 6*x * texel * tangent;
			fragToLight += 6*y * texel * coTangent;
			float currentDepth = length(fragToLight);  

	
			float value = texture(u_pointShadows, 
				vec4(fragToLight, light[index].castShadowsIndex),
				(currentDepth-bias)/light[index].dist ).r; 
			shadow += value;
		}
		
	}

	if(shadow <3)
	{
		shadow = 0;
	}

	shadow /= (kernel * kernel);


	//float samples = 5.0;
	//float offset  = 0.1;
	//for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	//{
	//	for(float y = -offset; y < offset; y += offset / (samples * 0.5))
	//	{
	//		for(float z = -offset; z < offset; z += offset / (samples * 0.5))
	//		{
	//			vec3 fragToLight = pos - light[index].positions; 
	//			float currentDepth = length(fragToLight + vec3(x,y,z));  
	//
	//
	//			float value = texture(u_pointShadows, 
	//				vec4(fragToLight + vec3(x, y, z), light[index].castShadowsIndex),
	//				(currentDepth-bias)/light[index].dist ).r; 
	//			shadow += value;
	//
	//		}
	//	}
	//}
	//shadow /= (samples * samples * samples);

	shadow = clamp(shadow, 0, 1);

	//if(shadow < 0.1)
	//{
	//	shadow = 0;
	//}

	return shadow;
}

float cascadedShadowCalculation(vec3 pos, vec3 normal, vec3 lightDir, int index)
{
	

	vec3 firstProjCoords = getProjCoords(dLight[index].firstLightSpaceMatrix, pos);
	vec3 secondProjCoords = getProjCoords(dLight[index].secondLightSpaceMatrix, pos);
	vec3 thirdProjCoords = getProjCoords(dLight[index].thirdLightSpaceMatrix, pos);


	if(
		firstProjCoords.x < 0.98 &&
		firstProjCoords.x > 0.01 &&
		firstProjCoords.y < 0.98 &&
		firstProjCoords.y > 0.01 &&
		firstProjCoords.z < 0.98 &&
		firstProjCoords.z > 0
	)
	{
		//return 0;
		firstProjCoords.y /= 3.f;

		return shadowCalculationLinear(firstProjCoords, normal, lightDir, u_cascades, index);
	}else 
	if(
		secondProjCoords.x > 0 &&
		secondProjCoords.x < 1 &&
		secondProjCoords.y > 0 &&
		secondProjCoords.y < 1 &&
		//secondProjCoords.z > 0 &&
		secondProjCoords.z < 0.98
	)
	{
		//return 1;
		secondProjCoords.y /= 3.f;
		secondProjCoords.y += 1.f / 3.f;

		return shadowCalculationLinear(secondProjCoords, normal, lightDir, u_cascades, index);
	}
	else
	{
		//return 2;
		thirdProjCoords.y /= 3.f;
		thirdProjCoords.y += 2.f / 3.f;

		return shadowCalculationLinear(thirdProjCoords, normal, lightDir, u_cascades, index);
	}

}


vec4 fromuShortToFloat2(ivec4 a)
{
	vec4 ret = a;

	//[0 65536] -> [0 1]
	ret /= 65536;

	//[0 1] -> [0 4]
	ret *= 4.f;

	//[0 4] -> [-2 2]
	ret -= 2.f;

	return ret;
}

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


//////////////////////////////////////////////
//https://imanolfotia.com/blog/1
//SSR


const float step = 0.1;
const float minRayStep = 0.1;
const float maxSteps = 30;
const int numBinarySearchSteps = 5;
const float reflectionSpecularFalloffExponent = 3.0;



//todo try this
//vec3 PositionFromDepth(float depth) {
//    float z = depth * 2.0 - 1.0;
//
//    vec4 clipSpacePosition = vec4(TexCoords * 2.0 - 1.0, z, 1.0);
//    vec4 viewSpacePosition = invprojection * clipSpacePosition;
//
//    // Perspective division
//    viewSpacePosition /= viewSpacePosition.w;
//
//    return viewSpacePosition.xyz;
//}

vec3 hash(vec3 a)
{
	const vec3 Scale = vec3(.8, .8, .8);
	const float K = 19.19;

	a = fract(a * Scale);
	a += dot(a, a.yxz + K);
	return fract((a.xxy + a.yxx)*a.zyx);
}


vec2 BinarySearch(inout vec3 dir, inout vec3 hitCoord, 
inout float dDepth, vec2 oldValue)
{
	float depth;

	vec4 projectedCoord;
	
	vec2 foundProjectedCoord = oldValue;

	for(int i = 0; i < numBinarySearchSteps; i++)
	{

		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
		//depth = textureLod(gPosition, projectedCoord.xy, 2).z;
		depth = texture(u_positionViewSpace, projectedCoord.xy).z;
 
		if(depth < -1000) //-INFINITY
			continue;

		foundProjectedCoord = projectedCoord.xy;

		dDepth = hitCoord.z - depth;

		dir *= 0.5;
		if(dDepth > 0.0)
			hitCoord += dir;
		else
			hitCoord -= dir;    
	}

		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
	
	if(!(depth < -1000))
	{
		foundProjectedCoord = projectedCoord.xy;
	}

	return foundProjectedCoord.xy;
}

vec2 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
	dir *= step;
 
	float depth;
	int steps;
	vec4 projectedCoord;
 
	for(int i = 0; i < maxSteps; i++)
	{
		hitCoord += dir;
 
		projectedCoord = u_cameraProjection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
		//depth = textureLod(gPosition, projectedCoord.xy, 2).z;
		depth = texture(u_positionViewSpace, projectedCoord.xy).z;

		if(depth > 1000.0)
			continue;
		
		if(depth < -1000) //-INFINITY
			continue;

		dDepth = hitCoord.z - depth;

		if((dir.z - dDepth) < 1.2)
		{
			if(dDepth <= 0.0)
			{   
				vec2 Result;
				Result = BinarySearch(dir, hitCoord, dDepth, projectedCoord.xy);
				//Result = projectedCoord.xy;
				return Result;
			}
		}
		
		steps++;
	}
 
	//signal fail	
	dDepth = -INFINITY;
	return vec2(0,0);
}


////////////////////////////////////////////

vec3 SSR(vec3 viewPos, vec3 N, float metallic, vec3 F, out bool found)
{
	// Reflection vector
	//vec3 viewPos = -V;//todo check again :)
	vec3 reflected = normalize(reflect(normalize(viewPos), N));
	//vec3 reflected = R; //todo check
	
	//found = true;
	//return viewPos;

	//if(reflected.z < 0){found = false; return vec3(0,0,0);}

	vec3 hitPos = viewPos;
	float dDepth;

	//vec3 wp = vec3(vec4(viewPos, 1.0) * invView);
	//vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), spec);
	
	//todo test
	//vec3 jitt = mix(vec3(0.0), vec3(hash(wp)), -roughness); //use roughness for specular factor
	vec3 jitt = vec3(0.0);

	vec2 coords = RayMarch((vec3(jitt) + reflected * max(minRayStep, -viewPos.z)), hitPos, dDepth);
	
	if(dDepth == -INFINITY){found = false; return vec3(0);}


	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));
 
	float screenEdgefactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

	float ReflectionMultiplier = pow(metallic, reflectionSpecularFalloffExponent) * 
			screenEdgefactor * 
			-reflected.z;
 
	// Get color
	vec3 lastFrameColor = textureLod(u_lastFrameTexture, coords.xy, 0).rgb;
	//vec3 SSR = lastFrameColor * clamp(ReflectionMultiplier, 0.0, 0.9) * F;  
	vec3 SSR = lastFrameColor;// * clamp(ReflectionMultiplier, 0.0, 0.9);  


	found = true;

	return SSR;
}


//normal, viewDir, wp = world space position
//F = freshnell
//todo send F 
vec3 computeAmbientTerm(vec3 N, vec3 V, vec3 F0, float roughness, vec3 R, 
float metallic, vec3 albedo, vec3 wp, vec3 viewPos)
{


	vec3 gammaAmbient = pow(lightPassData.ambientColor.rgb, vec3(2.2)); //just the static ambient color
	vec3 ambient = vec3(0);
		
		
	float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);
	vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
	vec3 kS = F;

	vec3 irradiance = vec3(0,0,0); //diffuse
	vec3 radiance = vec3(0,0,0); //specular
	vec2 brdf = vec2(0,0);
	vec2 brdfVec = vec2(dotNVClamped, roughness);

	bool foundRadianceValue = false;

	if(u_hasLastFrameTexture != 0) //ssr
	{
		radiance = SSR(viewPos, N, metallic, F, foundRadianceValue);

		if(!foundRadianceValue)
		{
		foundRadianceValue= true;
		radiance = vec3(0);
		}
	}

	if(lightPassData.skyBoxPresent != 0)
	{

		irradiance = texture(u_skyboxIradiance, N).rgb; //this color is coming directly at the object
		
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		const float MAX_REFLECTION_LOD = 4.0;

		if(!foundRadianceValue)
		{
			radiance = textureLod(u_skyboxFiltered, R, roughness * MAX_REFLECTION_LOD).rgb;
		}

		//brdfVec.y = 1 - brdfVec.y; 
		brdf  = texture(u_brdfTexture, brdfVec).rg;
	
	}else
	{
		irradiance = gammaAmbient ; //this color is coming directly at the object
		
		if(!foundRadianceValue)
		{
			radiance = gammaAmbient;
		}

		//brdfVec.y = 1 - brdfVec.y; 
		brdf = texture(u_brdfTexture, brdfVec).rg;
	}

	if(lightPassData.lightSubScater == 0)
	{
		vec3 kD = 1.0 - kS;
		kD *= 1.0 - metallic;
		
		vec3 diffuse = irradiance * albedo;
		
		vec3 specular = radiance * (F * brdf.x + brdf.y);

		//no multiple scattering
		ambient = (kD * diffuse + specular);
	}else
	{
		//http://jcgt.org/published/0008/01/03/
		// Multiple scattering version
		vec3 FssEss = kS * brdf.x + brdf.y;
		float Ess = brdf.x + brdf.y;
		float Ems = 1-Ess;
		vec3 Favg = F0 + (1-F0)/21;
		vec3 Fms = FssEss*Favg/(1-(1-Ess)*Favg);
		// Dielectrics
		vec3 Edss = 1 - (FssEss + Fms * Ems);
		vec3 kD = albedo * Edss;

		// Multiple scattering version
		ambient = FssEss * radiance + (Fms*Ems+kD) * irradiance;
	}

	if(lightPassData.skyBoxPresent != 0)
	{
		ambient *= gammaAmbient;
	}
	
	return ambient;
}

void main()
{
	int materialIndex = texture(u_materialIndex, v_texCoords).r;

	if(materialIndex == 0)
	{
		//no material, no alpha component that is important
		//discard ??
		//discard;
		if(u_transparentPass != 0)
		{
			discard;
		}else
		{
			a_outColor = vec4(0,0,0,0);
			a_outBloom = vec4(0,0,0,1);
			return;
		}
		//albedoAlpha = vec4(0,0,0,0);
	}

	vec3 pos = texture(u_positions, v_texCoords).xyz;
	vec3 posViewSpace = texture(u_positionViewSpace, v_texCoords).xyz;


	if(posViewSpace.z == -INFINITY){discard;}

	vec3 normal = fromuShortToFloat(texture(u_normals, v_texCoords).xyz);
	vec2 sampledUV = texture(u_textureUV, v_texCoords).xy;
	ivec4 sampledDerivatesInt = texture(u_textureDerivates, v_texCoords).xyzw;
	vec4 sampledDerivates = fromuShortToFloat2(sampledDerivatesInt);
	//vec4 sampledDerivates = texture(u_textureDerivates, v_texCoords).xyzw;


	vec4 albedoAlpha = vec4(0,0,0,0);
	vec3 emissive = vec3(0,0,0);
	

	//vec4 sampledDerivates = texture(u_textureDerivates, v_texCoords).xyzw;
	

	vec3 material = vec3(0,0,0);

	{
		uvec2 albedoSampler = mat[materialIndex-1].firstBIndlessSamplers.xy;
		if(albedoSampler.x == 0 && albedoSampler.y == 0)
		{
			albedoAlpha.rgba = vec4(1,1,1,1); //multiply after with color;
		}else
		{
			albedoAlpha = 
				textureGrad(sampler2D(albedoSampler), sampledUV.xy, 
					sampledDerivates.xy, sampledDerivates.zw).rgba;
		}

		albedoAlpha.rgb *= pow( vec3(mat[materialIndex-1].kd), vec3(1.0/2.2) );
		albedoAlpha.a *= mat[materialIndex-1].kd.a;

		uvec2 emmisiveSampler = mat[materialIndex-1].secondBIndlessSamplers.xy;
		if(emmisiveSampler.x == 0 && emmisiveSampler.y == 0)
		{
			emissive.rgb = albedoAlpha.rgb;
		}else
		{
			emissive = 
				textureGrad(sampler2D(emmisiveSampler), sampledUV.xy, 
					sampledDerivates.xy, sampledDerivates.zw).rgb;
		}

		emissive.rgb *= mat[materialIndex-1].rma.a;
		emissive = pow(emissive , vec3(2.2)).rgb; //gamma corection
		
		uvec2 rmaSampler = mat[materialIndex-1].firstBIndlessSamplers.zw;
		if(rmaSampler.x == 0 && rmaSampler.y == 0 && mat[materialIndex-1].rmaLoaded != 0)
		{
			material.r = mat[materialIndex-1].rma.r;
			material.g = mat[materialIndex-1].rma.g;
			material.b = mat[materialIndex-1].rma.b;
		}
		else
		{
			vec3 materialData = textureGrad(sampler2D(rmaSampler), sampledUV.xy, 
					sampledDerivates.xy, sampledDerivates.zw).rgb;

			int roughnessPrezent = mat[materialIndex-1].rmaLoaded & 0x4;
			int metallicPrezent = mat[materialIndex-1].rmaLoaded & 0x2;
			int ambientPrezent = mat[materialIndex-1].rmaLoaded & 0x1;

			if(roughnessPrezent != 0)
			{
				material.r = materialData.r;
			}else
			{
				material.r = mat[materialIndex-1].rma.r;
			}

			if(metallicPrezent != 0)
			{
				material.g = materialData.g;
			}else
			{
				material.g = mat[materialIndex-1].rma.g;
			}

			if(ambientPrezent != 0)
			{
				material.b = materialData.b;
			}else
			{
				material.b = mat[materialIndex-1].rma.b;
			}

		}

	}
	
	//calculate BRDF

	vec3 albedo = albedoAlpha.rgb;
	albedo  = pow(albedo , vec3(2.2)).rgb; //gamma corection

	float roughness = clamp(material.r, 0.09, 0.99);
	float metallic = clamp(material.g, 0.0, 0.98);
	float ambientOcclution = material.b;

	vec3 viewDir = normalize(u_eyePosition - pos); //towards hemisphere

	//vec3 I = normalize(pos - u_eyePosition); //looking direction (towards eye)
	vec3 R = reflect(-viewDir, normal);	//reflected vector
	//vec3 skyBoxSpecular = texture(u_skybox, R).rgb;		//this is the reflected color


	vec3 Lo = vec3(0,0,0); //this is the accumulated light

	

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo.rgb, vec3(metallic));

	//foreach point light
	for(int i=0; i<u_pointLightCount;i++)
	{
		vec3 lightPosition = light[i].positions.xyz;
		vec3 lightColor = light[i].color.rgb;
		vec3 lightDirection = normalize(lightPosition - pos);

		float currentDist = distance(lightPosition, pos);
		if(currentDist >= light[i].dist)
		{
			continue;
		}

		float attenuation = attenuationFunctionNotClamped(currentDist, light[i].dist, light[i].attenuation);	

		float shadow = 1.f;
		if(light[i].castShadows != 0)
		{
			shadow = pointShadowCalculation(pos, normal, i);
			shadow = pow(shadow, light[i].hardness);
		}

		Lo += computePointLightSource(lightDirection, metallic, roughness, lightColor, 
			pos, viewDir, albedo, normal, F0) * attenuation * shadow;

	}

	for(int i=0; i<u_directionalLightCount; i++)
	{
		
		vec3 lightDirection = dLight[i].direction.xyz;
		vec3 lightColor = dLight[i].color.rgb;

		float shadow = 1;
		
		if(dLight[i].castShadows != 0)
		{	
			int castShadowInd = dLight[i].castShadowsIndex;
			shadow = cascadedShadowCalculation(pos, normal, lightDirection, castShadowInd);
			shadow = pow(shadow, dLight[i].color.w);
		}

		//if(shadow == 0)
		//{
		//	albedo.rgb = vec3(1,0,0);
		//}else if(shadow >= 0.9 && shadow <= 1.1)
		//{
		//	albedo.rgb = vec3(0,1,0);
		//}else
		//{
		//	albedo.rgb = vec3(0,0,1);
		//}
		//shadow = 1;

		Lo += computePointLightSource(-lightDirection, metallic, roughness, lightColor, 
			pos, viewDir, albedo, normal, F0) * shadow;
	}

	for(int i=0; i<u_spotLightCount; i++)
	{
		vec3 lightPosition = spotLights[i].position.xyz;
		vec3 lightColor = spotLights[i].color.rgb;
		vec3 spotLightDirection = spotLights[i].direction.xyz;
		vec3 lightDirection = -normalize(lightPosition - pos);

		float angle = spotLights[i].position.w;
		float dist = spotLights[i].direction.w;
		float at = spotLights[i].color.w;

		float dotAngle = dot(normalize(vec3(pos - lightPosition)), spotLightDirection);

		float currentDist = distance(lightPosition, pos);

		if(currentDist >= dist)
		{
			continue;
		}

		if(dotAngle > angle && dotAngle > 0)
		{
			float attenuation = attenuationFunctionNotClamped(currentDist, dist, at);
			//attenuation = 1;

			float smoothingVal = 0.01; //
			float innerAngle = angle + smoothingVal;

			float smoothing = clamp((dotAngle-angle)/smoothingVal,0.0,1.0);
			//smoothing = 1;

			vec3 shadowProjCoords = getProjCoords(spotLights[i].lightSpaceMatrix, pos);
			
			float shadow = 1;
			
			if(spotLights[i].castShadows != 0)
			{
				shadow = shadowCalculationLogaritmic(shadowProjCoords, normal, lightDirection, 
					u_spotShadows, spotLights[i].shadowIndex, spotLights[i].near, spotLights[i].far);

				shadow = pow(shadow, spotLights[i].hardness);
			}


			smoothing = pow(smoothing, spotLights[i].hardness);

			Lo += computePointLightSource(-lightDirection, metallic, roughness, lightColor, 
				pos, viewDir, albedo, normal, F0) * smoothing * attenuation * shadow;
		}

	}


	//vec3 wp = vec3(vec4(pos, 1.0) * inverse(u_viewProj));
	vec3 wp = viewDir; //todo

	vec3 color = Lo + computeAmbientTerm(normal, viewDir, F0, roughness, R, metallic, 
	albedo, wp, posViewSpace) * ambientOcclution; 

	//vec3 hdrCorrectedColor = color;
	//hdrCorrectedColor.rgb = vec3(1.0) - exp(-hdrCorrectedColor.rgb  * lightPassData.exposure);
	//hdrCorrectedColor.rgb = pow(hdrCorrectedColor.rgb, vec3(1.0/2.2));
	//float lightIntensity = dot(hdrCorrectedColor.rgb, vec3(0.2126, 0.7152, 0.0722));	
	////float lightIntensity = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));	
	//gama correction and hdr is done and applied in the post process step

	if(u_transparentPass != 0)
	{
		a_outColor = vec4(color.rgb + emissive.rgb, albedoAlpha.a);
		a_outBloom = vec4(emissive.rgb, albedoAlpha.a);
	}else
	{
		a_outColor = vec4(color.rgb + emissive.rgb, 1);
		a_outBloom = vec4(emissive.rgb, 1);
	}
	
	if(u_hasLastFrameTexture!=0)
	{
		//vec3 lastFrameColor = texture(u_lastFrameTexture, v_texCoords).xyz;
		//a_outColor.rgb = 0.7 * a_outColor.rgb + 0.3 * lastFrameColor;
		//a_outColor.rgb = lastFrameColor;
	}



	//a_outColor.rgb = vec3(albedoAlpha);
	//a_outColor.rgb =  material.bbb;
	//a_outColor.rgba =  vec4(albedoAlpha.aaa, 1);
	//a_outColor.rgb = vec3(ssaof, ssaof, ssaof);

}
