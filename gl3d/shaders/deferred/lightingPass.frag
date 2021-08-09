#version 430
#pragma debug(on)

//#extension GL_NV_shadow_samplers_cube : enable

layout(location = 0) out vec4 a_outColor;
layout(location = 1) out vec4 a_outBloom;

in vec2 v_texCoords;


uniform sampler2D u_albedo;
uniform sampler2D u_normals;
uniform samplerCube u_skyboxFiltered;
uniform samplerCube u_skyboxIradiance;
uniform sampler2D u_positions;
uniform sampler2D u_materials;
uniform sampler2D u_brdfTexture;
uniform sampler2D u_emmisive;
uniform sampler2DArrayShadow u_cascades;


uniform vec3 u_eyePosition;
uniform mat4 u_view;

layout (std140) uniform u_lightPassData
{
	vec4 ambientColor;
	float bloomTresshold;
	int lightSubScater;
	float exposure;

}lightPassData;

struct PointLight
{
	vec3 positions; 
	float dist;
	vec3 color;
	float strength;
};
readonly layout(std140) buffer u_pointLights
{
	PointLight light[];
};
uniform int u_pointLightCount;


struct DirectionalLight
{
	vec4 direction; //w not used
	vec4 color;		//w is a hardness exponent
	mat4 firstLightSpaceMatrix;
	mat4 secondLightSpaceMatrix;
	mat4 thirdLightSpaceMatrix;

};
readonly layout(std140) buffer u_directionalLights
{
	DirectionalLight dLight[];
};
uniform int u_directionalLightCount;



const float PI = 3.14159265359;

const float randomNumbers[100] = {
0.05535,	0.22262,	0.93768,	0.80063,	0.40089,	0.49459,	0.44997,	0.27060,	0.58789,	0.61765,
0.87949,	0.38913,	0.23154,	0.27249,	0.93448,	0.71567,	0.26940,	0.32226,	0.73918,	0.30905,
0.98754,	0.82585,	0.84031,	0.60059,	0.56027,	0.10819,	0.55848,	0.95612,	0.88034,	0.94950,
0.53892,	0.86421,	0.84131,	0.39158,	0.25861,	0.10192,	0.19673,	0.25165,	0.68675,	0.79157,
0.94730,	0.36948,	0.27978,	0.66377,	0.38935,	0.93795,	0.83168,	0.01452,	0.51242,	0.12272,
0.61045,	0.34752,	0.13781,	0.92361,	0.73422,	0.31213,	0.55513,	0.81074,	0.56166,	0.31797,
0.09507,	0.50049,	0.44248,	0.38244,	0.58468,	0.32327,	0.61830,	0.67908,	0.16011,	0.82861,
0.36502,	0.12052,	0.28872,	0.73448,	0.51443,	0.99355,	0.75244,	0.22432,	0.95501,	0.90914,
0.37992,	0.61330,	0.49202,	0.69464,	0.14831,	0.51697,	0.34620,	0.55315,	0.41602,	0.49807,
0.15133,	0.07372,	0.75259,	0.59642,	0.35652,	0.60051,	0.08879,	0.59271,	0.29388,	0.69505,
};


//n normal
//h halfway vector
//a roughness	(1 rough, 0 glossy) 
//this gets the amount of specular light reflected
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
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
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
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

vec3 computePointLightSource(vec3 lightDirection, float metallic, float roughness, in vec3 lightColor, in vec3 worldPosition,
	in vec3 viewDir, in vec3 color, in vec3 normal, in vec3 F0)
{
	//lightDirection = direction to light
	//vec3 lightDirection = normalize(lightPosition - worldPosition);

	float dotNVclamped = clamp(dot(normal, viewDir), 0.0, 0.99);

	vec3 halfwayVec = normalize(lightDirection + viewDir);
	
	//float dist = length(lightPosition - worldPosition);
	//float attenuation = 1.0 / pow(dist,2);
	float attenuation = 1; //(option) remove light attenuation
	vec3 radiance = lightColor * attenuation; //here the first component is the light color
	
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	float NDF = DistributionGGX(normal, halfwayVec, roughness);       
	float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   

	float denominator = 4.0 * dotNVclamped  
		* max(dot(normal, lightDirection), 0.0);
	vec3 specular     = (NDF * G * F) / max(denominator, 0.001);

	vec3 kS = F; //this is the specular contribution
	vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
	kD *= 1.0 - metallic;	//metallic surfaces are darker
	
	
	float NdotL = max(dot(normal, lightDirection), 0.0);        
	vec3 Lo = (kD * color.rgb / PI + specular) * radiance * NdotL;

	return Lo;
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

//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-17-efficient-soft-edged-shadows-using
float shadowCalculation(vec3 projCoords, vec3 normal, vec3 lightDir, sampler2DArrayShadow shadowMap, int index)
{

	// keep the shadow at 1.0 when outside or close to the far_plane region of the light's frustum.
	if(projCoords.z > 0.9995)
		return 1.f;
	//if(projCoords.z < 0)
	//	return 1.f;

	//float closestDepth = texture(shadowMap, projCoords.xy).r; 
	float currentDepth = projCoords.z;


	float bias = max((10.f/1024.f) * (1.0 - dot(normal, -lightDir)), 3.f/1024.f);
	//float bias = 0.1;
	
	//todo move
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
	float shadow = 0.0;

	bool fewSamples = false;
	int kernelSize = 5;
	int kernelSize2 = kernelSize*kernelSize;
	int kernelHalf = kernelSize/2;

	float shadowValueAtCentre = 0;

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
			float s2 = testShadowValue(shadowMap, projCoords.xy + offsets[i] * texelSize, 
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


		for(int x = -kernelHalf; x <= kernelHalf; ++x)
		{
			for(int y = -kernelHalf; y <= kernelHalf; ++y)
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
					float u = (offset.x + kernelHalf+1)/float(kernelSize);
					float v = (offset.x + kernelHalf+1)/float(kernelSize);
					offset.x = sqrt(v) * cos(2*PI * u)* kernelSize / 2.f;
					offset.y = sqrt(v) * sin(2*PI * u)* kernelSize / 2.f;
				}

				float s = testShadowValue(shadowMap, projCoords.xy + offset * texelSize, 
					currentDepth, bias, index); 
				
				//float s = sampleShadowLinear(shadowMap, projCoords.xy + vec2(x, y) * offset,
				//	texelSize, currentDepth, bias); 
				
				shadow += s;
			}    
		}
		shadow /= kernelSize2;
	
	}

	shadow = pow(shadow, dLight[index].color.w);
	
	return shadow;
}

vec3 getProjCoords(in mat4 matrix, in vec3 pos)
{
	vec4 p = matrix * vec4(pos,1);
	vec3 r = p .xyz / p .w;
	r = r * 0.5 + 0.5;
	return r;
}

float cascadedShadowCalculation(vec3 pos, vec3 normal, vec3 lightDir, int index)
{
	
	vec4 viewSpacePos = u_view * vec4(pos, 1);
	float depth = -viewSpacePos.z; //zfar

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

		return shadowCalculation(firstProjCoords, normal, lightDir, u_cascades, index);
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

		return shadowCalculation(secondProjCoords, normal, lightDir, u_cascades, index);
	}
	else
	{
		//return 2;
		thirdProjCoords.y /= 3.f;
		thirdProjCoords.y += 2.f / 3.f;

		return shadowCalculation(thirdProjCoords, normal, lightDir, u_cascades, index);
	}

}

float linStep(float v, float low, float high)
{
	return clamp((v-low) / (high-low), 0.0, 1.0);

};

//https://www.youtube.com/watch?v=LGFDifcbsoQ&ab_channel=thebennybox
//float varianceShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
//{
//	//transform to depth buffer coords
//	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//	projCoords = projCoords * 0.5 + 0.5;
//
//	// keep the shadow at 1.0 when outside or close to the far_plane region of the light's frustum.
//	if(projCoords.z > 0.99)
//		return 1.f;
//	if(projCoords.z < 0)
//		return 1.f;
//	if(projCoords.x < 0 || projCoords.y < 0 || projCoords.x > 1 || projCoords.y > 1)
//		return 1.f;
//
//
//	vec2 sampled = texture(u_directionalShadow, projCoords.xy).rg; 
//	float closestDepth = sampled.r; 
//	float closestDepthSquared = sampled.g; 
//	float currentDepth = projCoords.z;
//
//
//	float bias = max(0.3 * (1.0 - dot(normal, -lightDir)), 0.09);
//	//float bias = 0.0;
//	
//	//float shadow = step(currentDepth-bias, closestDepth);       
//	float variance = max(closestDepthSquared - closestDepth*closestDepth, 0.00002);
//
//	float d = currentDepth - closestDepth; //distanceFromMean
//	float pMax = linStep(variance / (variance+ d*d), bias, 1); 
//
//
//	return min(max(d, pMax), 1.0);
//
//}



void main()
{
	vec3 pos = texture(u_positions, v_texCoords).xyz;
	vec3 normal = texture(u_normals, v_texCoords).xyz;
	vec4 albedoAlpha = texture(u_albedo, v_texCoords).rgba;
	vec3 emissive = texture(u_emmisive, v_texCoords).xyz;

	vec3 albedo = albedoAlpha.rgb;

	albedo  = pow(albedo , vec3(2.2,2.2,2.2)).rgb; //gamma corection
	emissive  = pow(emissive , vec3(2.2,2.2,2.2)).rgb; //gamma corection


	vec3 material = texture(u_materials, v_texCoords).xyz;



	vec3 viewDir = normalize(u_eyePosition - pos);

	//vec3 I = normalize(pos - u_eyePosition); //looking direction (towards eye)
	vec3 R = reflect(-viewDir, normal);	//reflected vector
	//vec3 skyBoxSpecular = texture(u_skybox, R).rgb;		//this is the reflected color
	vec3 skyBoxDiffuse = texture(u_skyboxIradiance, normal).rgb; //this color is coming directly to the object


	vec3 Lo = vec3(0,0,0); //this is the accumulated light

	float roughness = clamp(material.r, 0.09, 0.99);
	float metallic = clamp(material.g, 0.0, 0.98);
	float ambientOcclution = material.b;

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo.rgb, vec3(metallic));

	//foreach point light
	for(int i=0; i<u_pointLightCount;i++)
	{
		vec3 lightPosition = light[i].positions.xyz;
		vec3 lightColor = light[i].color.rgb;
		vec3 lightDirection = normalize(lightPosition - pos);

		Lo += computePointLightSource(lightDirection, metallic, roughness, lightColor, 
			pos, viewDir, albedo, normal, F0);

	}

	for(int i=0; i<u_directionalLightCount; i++)
	{
		
		vec3 lightDirection = normalize(dLight[i].direction.xyz);
		vec3 lightColor = dLight[i].color.rgb;

		float shadow = cascadedShadowCalculation(pos, normal, lightDirection, i);
		
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


	//compute ambient
	vec3 ambient;
	{

		vec3 N = normal;
		vec3 V = viewDir;
		
		float dotNVClamped = clamp(dot(N, V), 0.0, 0.99);

		vec3 F = fresnelSchlickRoughness(dotNVClamped, F0, roughness);
		vec3 kS = F;
		
		vec3 irradiance = skyBoxDiffuse;
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		const float MAX_REFLECTION_LOD = 4.0;
		vec3 radiance = textureLod(u_skyboxFiltered, R, roughness * MAX_REFLECTION_LOD).rgb;

		vec2 brdfVec = vec2(dotNVClamped, roughness);
		//brdfVec.y = 1 - brdfVec.y; 
		vec2 brdf  = texture(u_brdfTexture, brdfVec).rg;


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
		
		vec3 occlusionData = ambientOcclution * lightPassData.ambientColor.rgb;
		ambient *= occlusionData;

	}

	vec3 color = Lo + ambient; 

	

	vec3 hdrCorrectedColor = color;
	hdrCorrectedColor.rgb = vec3(1.0) - exp(-hdrCorrectedColor.rgb  * lightPassData.exposure);
	hdrCorrectedColor.rgb = pow(hdrCorrectedColor.rgb, vec3(1.0/2.2));


	float lightIntensity = dot(hdrCorrectedColor.rgb, vec3(0.2126, 0.7152, 0.0722));	

	//gama correction and hdr is done in the post process step

	//if(albedoAlpha.a == 0) discard;

	if(lightIntensity > lightPassData.bloomTresshold)
	{
		//a_outBloom = clamp(vec4(color.rgb, 1), 0, 1) + vec4(emissive.rgb, 0);
		//a_outColor = clamp(vec4(color.rgb, 1), 0, 1);	

		a_outBloom = vec4(color.rgb, 0) + vec4(emissive.rgb, 0);
		//a_outColor = vec4(color.rgb, albedoAlpha.a);	
		a_outColor = vec4(0,0,0, albedoAlpha.a);	

	}else
	{
		//a_outBloom = vec4(0, 0, 0, 0) + vec4(emissive.rgb, 0); //note (vlod) keep this here
		//a_outColor = clamp(vec4(color.rgb, 1), 0, 1);

		a_outBloom = vec4(emissive.rgb, 0);
		a_outColor = vec4(color.rgb, albedoAlpha.a);
	}

	a_outBloom = clamp(a_outBloom, 0, 1000);
	
	//a_outColor.rgb =  material.bbb;
	//a_outColor.rgba =  vec4(albedoAlpha.aaa, 1);
	//a_outColor.rgb = vec3(ssaof, ssaof, ssaof);

}
