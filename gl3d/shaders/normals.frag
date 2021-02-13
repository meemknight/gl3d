#version 430
#pragma debug(on)

#extension GL_NV_shadow_samplers_cube : enable

out layout(location = 0) vec4 a_outColor;

in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;

uniform vec3 u_lightPosition;
uniform vec3 u_eyePosition;

uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;
uniform samplerCube u_skybox;
uniform float u_gama;

uniform sampler2D u_RMASampler;

struct Pointlight
{
	vec4 positions; // w component not used
	vec4 color; // w component not used
};

layout(std140) buffer u_pointLights
{
	Pointlight light[];
};

uniform int u_pointLightCount;

//uniform Pointlight u_pointLights;

layout(std140) uniform u_material
{
	vec4 kd; //= 0.45;//w component not used
	float roughness;
	float metallic;
	float ao; //one means full light
};

//todo move some of this from global for implementing multi lights
vec3 normal; //the normal of the object (can be normal mapped or not)
vec3 noMappedNorals; //this is the original non normal mapped normal
vec3 viewDir;
float difuseTest;  // used to check if object is in the light
vec4 color; //texture color

float PI = 3.14159265359;

//https://gamedev.stackexchange.com/questions/22204/from-normal-to-rotation-matrix#:~:text=Therefore%2C%20if%20you%20want%20to,the%20first%20and%20second%20columns.
mat3x3 NormalToRotation(in vec3 normal)
{
	// Find a vector in the plane
	vec3 tangent0 = cross(normal, vec3(1, 0, 0));
	if (dot(tangent0, tangent0) < 0.001)
		tangent0 = cross(normal, vec3(0, 1, 0));
	tangent0 = normalize(tangent0);
	// Find another vector in the plane
	vec3 tangent1 = normalize(cross(normal, tangent0));
	// Construct a 3x3 matrix by storing three vectors in the columns of the matrix

	return mat3x3(tangent0,tangent1,normal);

	//return ColumnVectorsToMatrix(tangent0, tangent1, normal);
}

subroutine vec3 GetNormalMapFunc(vec3);

subroutine (GetNormalMapFunc) vec3 normalMapped(vec3 v)
{
	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(v);
	normal = rotMat * normal;
	normal = normalize(normal);
	return normal;
}

subroutine (GetNormalMapFunc) vec3 noNormalMapped(vec3 v)
{
	return v;
}

subroutine uniform GetNormalMapFunc getNormalMapFunc;


subroutine vec3 GetMaterialMapped();

subroutine (GetMaterialMapped) vec3 materialNone()
{
	return vec3(roughness, metallic, ao);
}

subroutine (GetMaterialMapped) vec3 materialR()
{
	float r = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(r, metallic, ao);
}

subroutine (GetMaterialMapped) vec3 materialM()
{
	float m = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(roughness, m, ao);
}

subroutine (GetMaterialMapped) vec3 materialA()
{
	float a = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(roughness, metallic, a);
}

subroutine (GetMaterialMapped) vec3 materialRM()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).rg;
	return vec3(v.x, v.y, ao);
}

subroutine (GetMaterialMapped) vec3 materialRA()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).rb;
	return vec3(v.x, metallic, v.y);
}

subroutine (GetMaterialMapped) vec3 materialMA()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).gb;
	return vec3(roughness, v.x, v.y);
}

subroutine (GetMaterialMapped) vec3 materialRMA()
{
	return texture2D(u_RMASampler, v_texCoord).rgb;
}

subroutine uniform GetMaterialMapped u_getMaterialMapped;



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

vec3 computePointLightSource(vec3 lightPosition, float metallic, float roughness, in vec3 lightColor)
{


	vec3 lightDirection = normalize(lightPosition - v_position);
	vec3 halfwayVec = normalize(lightDirection + viewDir);
	
	float dist = length(lightPosition - v_position);
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

	//vec3 sampledMaterial = texture2D(u_RMASampler, v_texCoord).rgb;
	vec3 sampledMaterial = u_getMaterialMapped();

	float roughnessSampled = sampledMaterial.r;
	roughnessSampled = max(0.50,roughnessSampled);

	float metallicSampled = sampledMaterial.g;
	float sampledAo = sampledMaterial.b;

	{	//general data
		color = texture2D(u_albedoSampler, v_texCoord).xyzw;
		if(color.w <= 0.1)
			discard;

		color.rgb = pow(color.rgb, vec3(2.2,2.2,2.2)).rgb; //gamma corection
		
		color *= vec4(kd.rgb,1); //(option) multiply texture by kd
		

		//color = vec4(kd.rgb,1); //(option) remove albedo texture
	
		noMappedNorals = normalize(v_normals);
		normal = getNormalMapFunc(noMappedNorals);
		//normal = noMappedNorals; //(option) remove normal mapping


		viewDir = u_eyePosition - v_position;
		viewDir = normalize(viewDir); //v

		//difuseTest = dot(noMappedNorals, lightDirection); // used to check if object is in the light
		
	}


	vec3 I = normalize(v_position - u_eyePosition); //looking direction (towards eye)
	vec3 R = reflect(I, normal);	//reflected vector
	vec3 skyBoxSpecular = textureCube(u_skybox, R).rgb;		//this is the reflected color
	vec3 skyBoxDiffuse = textureCube(u_skybox, normal).rgb; //this color is coming directly to the object

	vec3 Lo = vec3(0,0,0); //this is the accumulated light


	//foreach point light
	for(int i=0; i<u_pointLightCount;i++)
	{
		vec3 lightPosition = light[i].positions.xyz;
		vec3 lightColor = light[i].color.rgb;

		Lo += computePointLightSource(lightPosition, metallicSampled, roughnessSampled, lightColor);
	}


	vec3 ambient = vec3(0.01) * color.rgb * sampledAo; //this value is made up
	vec3 color   = Lo + ambient; 

	 //HDR 
	//color = color / (color + vec3(1.0));
	float exposure = 1;
	color = vec3(1.0) - exp(-color  * exposure);
	
	//gamma correction
	color = pow(color, vec3(1.0/2.2));

	//color = clamp(color, 0, 1);

	
	//specularVec *= skyBoxSpecular;

	//ambientVec *= mix(skyBoxSpecular.rgb, skyBoxIntensity.rgb, 0.5);
	//ambientVec = mix(ambientVec, skyBoxSpecular, 0.5);
	//ambientVec *= skyBoxSpecular;
	
	//color.rgb *= (clamp(ambientVec + difuseVec, 0, 1) + specularVec);


	//vec3 caleidoscop = mix(skyBoxSpecular, color.rgb, cross(normal, viewDir)); 
	//float dotNormalEye = dot(normal, viewDir);
	//dotNormalEye = clamp(dotNormalEye, 0, 1);
	
	//color.rgb = mix(skyBoxSpecular, color.rgb, pow(dotNormalEye, 1/1.0) ); //90 degrees, 0, reflected color

	//color = caleidoscop;

	a_outColor = clamp(vec4(color.rgb,1), 0, 1);
	//a_outColor = vec4(sampledMaterial.rgb,1);
	

}