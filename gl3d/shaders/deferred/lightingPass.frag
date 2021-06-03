#version 430
#pragma debug(on)

//#extension GL_NV_shadow_samplers_cube : enable

layout(location = 0) out vec4 a_outColor;
layout(location = 1) out vec4 a_outBloom;

in vec2 v_texCoord;


uniform sampler2D u_albedo;
uniform sampler2D u_normals;
uniform samplerCube u_skybox;
uniform sampler2D u_positions;
uniform sampler2D u_materials;
uniform sampler2D u_ssao;

uniform vec3 u_eyePosition;
uniform mat4 u_view;

uniform int u_useSSAO;

layout (std140) uniform u_lightPassData
{
	vec4 ambientColor;
	float bloomTresshold;

}lightPassData;

struct Pointlight
{
	vec3 positions; // w component not used
	float dist;
	vec3 color; // w component not used
	float strength;
};

readonly layout(std140) buffer u_pointLights
{
	Pointlight light[];
};

uniform int u_pointLightCount;


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

vec3 computePointLightSource(vec3 lightPosition, float metallic, float roughness, in vec3 lightColor, in vec3 worldPosition,
	in vec3 viewDir, in vec3 color, in vec3 normal)
{

	vec3 lightDirection = normalize(lightPosition - worldPosition);
	vec3 halfwayVec = normalize(lightDirection + viewDir);
	
	float dist = length(lightPosition - worldPosition);
	float attenuation = 1.0 / pow(dist,2);
	attenuation = 1; //(option) remove light attenuation
	vec3 radiance = lightColor * attenuation; //here the first component is the light color
	
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, color.rgb, metallic);	//here color is albedo, metalic surfaces use albdeo
	vec3 F  = fresnelSchlick(max(dot(halfwayVec, viewDir), 0.0), F0);

	float NDF = DistributionGGX(normal, halfwayVec, roughness);       
	float G   = GeometrySmith(normal, viewDir, lightDirection, roughness);   

	float denominator = 4.0 * max(dot(normal, viewDir), 0.0)  
		* max(dot(normal, lightDirection), 0.0);
	vec3 specular     = (NDF * G * F) / max(denominator, 0.001);

	vec3 kS = F; //this is the specular contribution
	vec3 kD = vec3(1.0) - kS; //the difuse is the remaining specular
	kD *= 1.0 - metallic;	//metallic surfaces are darker
	
	// scale light by NdotL
	float NdotL = max(dot(normal, lightDirection), 0.0);        
	vec3 Lo = (kD * color.rgb / PI + specular) * radiance * NdotL;

	return Lo;
}


void main()
{
	vec3 pos = texture(u_positions, v_texCoord).xyz;
	vec3 normal = texture(u_normals, v_texCoord).xyz;
	vec3 albedo = texture(u_albedo, v_texCoord).xyz;
	albedo  = pow(albedo , vec3(2.2,2.2,2.2)).rgb; //gamma corection


	vec3 material = texture(u_materials, v_texCoord).xyz;

	float ssaof;

	if(u_useSSAO != 0)
	{
		ssaof = texture(u_ssao, v_texCoord).r;	
	}else
	{
		ssaof = 1;
	}

	float ssao_ambient = pow(ssaof, 6.4);
	float ssao_finalColor = pow(ssaof, 5);


	vec3 viewDir = normalize(u_eyePosition - pos);

	//note skybox is not binded
	//vec3 I = normalize(pos - u_eyePosition); //looking direction (towards eye)
	//vec3 R = reflect(I, normal);	//reflected vector
	//vec3 skyBoxSpecular = textureCube(u_skybox, R).rgb;		//this is the reflected color
	//vec3 skyBoxDiffuse = textureCube(u_skybox, normal).rgb; //this color is coming directly to the object


	vec3 Lo = vec3(0,0,0); //this is the accumulated light

	//foreach point light
	for(int i=0; i<u_pointLightCount;i++)
	{
		vec3 lightPosition = light[i].positions.xyz;
		vec3 lightColor = light[i].color.rgb;

		Lo += computePointLightSource(lightPosition, material.g, material.r, lightColor, 
			pos, viewDir, albedo, normal);

	}

	//vec3 ambientColor = vec3(0.03); //this value is made up
	vec3 ambientColor = lightPassData.ambientColor.rgb;
	
	ambientColor.rgb *= ssao_ambient;
	vec3 ambient = ambientColor * albedo.rgb * material.b; 
	vec3 color   = Lo + ambient; 

	color *= ssao_finalColor;


	float lightIntensity = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));	

	//HDR 
	//float exposure = 1;
	//color = color / (color + vec3(1.0));
	//color = vec3(1.0) - exp(-color  * exposure);
	
	
	//gama correction and hdr is done in the post process step


	//color.rgb =  material.bbb;

	//todo uniform for this thresshold or sthing
	if(lightIntensity > lightPassData.bloomTresshold)
	{
		a_outBloom = clamp(vec4(color.rgb, 1), 0, 1);
		a_outColor = clamp(vec4(color.rgb, 1), 0, 1);	

	}else
	{
		a_outBloom = vec4(0, 0, 0, 0); //note (vlod) keep this here
		a_outColor = clamp(vec4(color.rgb, 1), 0, 1);
	}


	//a_outColor.rgb = vec3(ssaof, ssaof, ssaof);

}