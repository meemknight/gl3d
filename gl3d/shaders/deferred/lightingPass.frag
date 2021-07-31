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
uniform sampler2D u_directionalShadow;


uniform vec3 u_eyePosition;
uniform mat4 u_view;

layout (std140) uniform u_lightPassData
{
	vec4 ambientColor;
	float bloomTresshold;
	int lightSubScater;

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
	vec4 color;		//w not used
	mat4 lightSpaceMatrix;
};
readonly layout(std140) buffer u_directionalLights
{
	DirectionalLight dLight[];
};
uniform int u_directionalLightCount;



const float PI = 3.14159265359;

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

float testShadowValue(sampler2D map, vec2 coords, float currentDepth, float bias)
{
	float closestDepth = texture(u_directionalShadow, coords).r; 

	return  (currentDepth - bias) < closestDepth  ? 1.0 : 0.0;
}

//https://www.youtube.com/watch?v=yn5UJzMqxj0&ab_channel=thebennybox
float sampleShadowLinear(sampler2D map, vec2 coords, vec2 texelSize, float currentDepth, float bias)
{

	vec2 pixelPos = coords / texelSize + vec2(0.5);
	vec2 fracPart = fract(pixelPos);
	vec2 startTexel = (pixelPos-fracPart) * texelSize;

	float blTexture = testShadowValue(map, startTexel, currentDepth, bias).r;
	float brTexture = testShadowValue(map, startTexel + vec2(texelSize.x, 0), currentDepth, bias).r;
	float tlTexture = testShadowValue(map, startTexel + vec2(0, texelSize.y), currentDepth, bias).r;
	float trTexture = testShadowValue(map, startTexel + texelSize, currentDepth, bias).r;

	float mixA = mix(blTexture, tlTexture, fracPart.y);
	float mixB = mix(brTexture, trTexture, fracPart.y);

	return mix(mixA, mixB, fracPart.x);
}



float shadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	//transform to depth buffer coords
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	// keep the shadow at 1.0 when outside the far_plane region of the light's frustum.
	if(projCoords.z > 1.0)
		return 1.f;


	float closestDepth = texture(u_directionalShadow, projCoords.xy).r; 
	float currentDepth = projCoords.z;


	float bias = max((3.f/1024.f) * (1.0 - dot(normal, lightDir)), 2.f/1024.f);
	//float bias = 0.1;
	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_directionalShadow, 0);
	vec2 offset = texelSize;
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			
			//float s = testShadowValue(u_directionalShadow, projCoords.xy + vec2(x, y) * offset, 
			//	currentDepth, bias); 
			
			float s = sampleShadowLinear(u_directionalShadow, projCoords.xy + vec2(x, y) * offset,
				texelSize, currentDepth, bias); 
			
			shadow += s;
		}    
	}
	shadow /= 9.0;
	
	//float shadow = (currentDepth - bias) < closestDepth  ? 1.0 : 0.0;        


	//shadow = pow(shadow, 4);
	
	return shadow;
}



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

		vec4 fragPosLightSpace = dLight[i].lightSpaceMatrix * vec4(pos,1);

		float shadow = shadowCalculation(fragPosLightSpace, normal, lightDirection);

		Lo += computePointLightSource(-lightDirection, metallic, roughness, lightColor, 
			pos, viewDir, albedo, normal, F0) * shadow;
	}



	vec3 ambient;
	//compute ambient
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

	

	float lightIntensity = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));	

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
