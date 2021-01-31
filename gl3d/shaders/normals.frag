#version 330
#pragma debug(on)

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


layout(std140) uniform u_material
{
	vec4 ka; //= 0.5; //w component not used
	vec4 kd; //= 0.45;//w component not used
	vec4 ks; //= 1;	 ;//w component is the specular exponent
	float roughness;
	float metallic;
	float ao;
};

vec3 normal; //the normal of the object (can be normal mapped or not)
vec3 noMappedNorals; //this is the original non normal mapped normal
vec3 lightDirection;
vec3 eyeDirection;
float difuseTest;  // used to check if object is in the light
vec4 color; //texture color

float PI = 3.14159;

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

vec3 applyGama(in vec3 v)
{
	return v.rgb = pow(v.rgb, vec3(1.0/u_gama));
}


vec3 getNormalMap(in vec3 v)
{
	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(v);
	normal = rotMat * normal;
	normal = normalize(normal);
	return normal;
}

//n normal
//h halfway vector
//a roughness	(1 rough, 0 glossy) 
//this gets the amount of specular light reflected
float DistributionGGX(vec3 N, vec3 H, float a)
{
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
	//float r = (roughness + 1.0); this is how you get k
	//float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return nom / denom;
}
 
//oclude light that is hidded begind small geometry roughnesses
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);
	
	return ggx1 * ggx2;
}


//cosTheta is the dot between the normal and halfway
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 computeSpecular()
{
	//Blinn-Phong
	vec3 halfwayDir = normalize(lightDirection + eyeDirection);
	float cosTheta = dot(halfwayDir, normal);

	//phong
	//vec3 reflectedLightVector = reflect(-lightDirection, normal);
	//float specularLight = dot(reflectedLightVector, eyeDirection);

	//if(difuseTest <= 0.01) 
	//{return vec3(0);}

	cosTheta = max(cosTheta,0);
	cosTheta = pow(cosTheta, ks.w);
	
	//return cosTheta * vec3(ks);

	float roughDistribution = DistributionGGX(normal, halfwayDir, roughness);
	return roughDistribution * vec3(ks);
}

vec3 computeDifuse()
{

	float difuseLight = dot(normal, lightDirection);
	difuseLight = max(difuseLight, 0);
	return vec3(kd) * difuseLight;
}



void main()
{
	{	//general data

		color = texture2D(u_albedoSampler, v_texCoord).xyzw;
		color = vec4(1); //(option) remove albedo texture
		if(color.w <= 0.1)
			discard;

		noMappedNorals = normalize(v_normals);
		normal = getNormalMap(noMappedNorals);
		normal = noMappedNorals; //(option) remove normal mapping

		lightDirection = u_lightPosition - v_position;
		lightDirection = normalize(lightDirection);

		eyeDirection = u_eyePosition - v_position;
		eyeDirection = normalize(eyeDirection);

		difuseTest = dot(noMappedNorals, lightDirection); // used to check if object is in the light
		
	}

	vec3 I = normalize(v_position - u_eyePosition); //looking direction (towards eye)
	vec3 R = reflect(I, normal);	//reflected vector
	vec3 skyBoxSpecular = textureCube(u_skybox, R).rgb;		//this is the reflected color
	vec3 skyBoxDiffuse = textureCube(u_skybox, normal).rgb; //this color is coming directly to the object


	vec3 difuseVec = computeDifuse();
	vec3 specularVec = computeSpecular();
	vec3 ambientVec = vec3(ka);
	
	//specularVec *= skyBoxSpecular;

	//ambientVec *= mix(skyBoxSpecular.rgb, skyBoxIntensity.rgb, 0.5);
	//ambientVec = mix(ambientVec, skyBoxSpecular, 0.5);
	//ambientVec *= skyBoxSpecular;
	
	color.rgb *= (clamp(ambientVec + difuseVec, 0, 1) + specularVec);


	vec3 caleidoscop = mix(skyBoxSpecular, color.rgb, cross(normal, eyeDirection)); 
	float dotNormalEye = dot(normal, eyeDirection);
	dotNormalEye = clamp(dotNormalEye, 0, 1);
	
	//color.rgb = mix(skyBoxSpecular, color.rgb, pow(dotNormalEye, 1/1.0) ); //90 degrees, 0, reflected color

	//color = caleidoscop;

	a_outColor = clamp(vec4(color.rgb,1), 0, 1);

	

}