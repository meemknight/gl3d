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

};


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


void main()
{
	vec3 noMappedNorals = normalize(v_normals);

	//float ka = 0.5;
	//float kd = 0.45;
	//float ks = 1;

	//get normal map data
	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(noMappedNorals);
	normal = rotMat * normal;
	normal = normalize(normal);
	//normal = noMappedNorals; //remove normal mapping

	vec3 I = normalize(v_position - u_eyePosition);
	vec3 R = reflect(I, normal);
	vec3 skyBoxSpecular = textureCubeLod(u_skybox, R, 0).rgb;
	vec3 skyBoxDiffuse = textureCubeLod(u_skybox, normal, 0).rgb; //todo


	vec3 lightDirection = u_lightPosition - v_position;
	lightDirection = normalize(lightDirection);

	float difuseTest = dot(noMappedNorals, lightDirection);
	float difuseLight = dot(normal, lightDirection);
	difuseLight = max(difuseLight, 0);

	vec3 eyeDirection = u_eyePosition - v_position;
	eyeDirection = normalize(eyeDirection);
	
	//Blinn-Phong
	vec3 halfwayDir = normalize(lightDirection + eyeDirection);
	float specularLight = dot(halfwayDir, normal);

	if(difuseTest <= 0.001) specularLight = 0;
	specularLight = max(specularLight,0);
	specularLight = pow(specularLight, ks.w);

	//vec3 reflectedLightVector = reflect(-lightDirection, normal);
	//float specularLight = dot(reflectedLightVector, eyeDirection);
	//specularLight = max(specularLight,0);
	//
	//specularLight = pow(specularLight, 32);
	//specularLight *= ks;
	//if(difuseTest <= 0.001) specularLight = 0;

	vec3 color = texture2D(u_albedoSampler, v_texCoord).xyz;
	vec3 difuseVec =  vec3(kd) * difuseLight;
	vec3 ambientVec = vec3(ka);
	vec3 specularVec = vec3(ks) * specularLight;
	
	specularVec *= skyBoxSpecular;

	//ambientVec *= mix(skyBoxSpecular.rgb, skyBoxIntensity.rgb, 0.5);
	//ambientVec = mix(ambientVec, skyBoxSpecular, 0.5);
	//ambientVec *= skyBoxSpecular;
	
	color *= (clamp(ambientVec + difuseVec, 0, 1) + specularVec);
	vec3 caleidoscop = mix(skyBoxSpecular, color, cross(normal, eyeDirection)); 
	float dotNormalEye = dot(normal, eyeDirection);
	dotNormalEye = clamp(dotNormalEye, 0, 1);
	

	//color = mix(skyBoxSpecular, color, pow(dotNormalEye, 1/1.0) ); //90 degrees, 0, reflected color

	//color = caleidoscop;

	a_outColor = clamp(vec4(color,1), 0, 1);



}