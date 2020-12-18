#version 330

out layout(location = 0) vec4 a_outColor;

in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;

uniform vec3 u_lightPosition;
uniform vec3 u_eyePosition;

uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;
uniform samplerCube u_skybox;


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

void main()
{


	float ka = 1;
	float kd = 0.45;
	float ks = 1;

	//get normal map data
	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(v_normals);
	normal = rotMat * normal;
	//normal = v_normals;

	vec3 I = normalize(v_position - u_eyePosition);
	vec3 R = reflect(I, normalize(normal));
	vec3 skyBoxColor = texture(u_skybox, R).rgb;
	vec3 skyBoxIntensity = vec3((skyBoxColor.r * skyBoxColor.g + skyBoxColor.b) /3);


	vec3 lightDirection = u_lightPosition - v_position;
	lightDirection = normalize(lightDirection);

	float difuseTest = dot(v_normals, lightDirection);
	float difuseLight = dot(normal, lightDirection);
	difuseLight = max(difuseLight, 0);
	difuseLight *= kd;

	vec3 eyeDirection = u_eyePosition - v_position;
	eyeDirection = normalize(eyeDirection);
	
	//todo fix
	vec3 reflectedLightVector = reflect(-lightDirection, normal);
	float specularLight = dot(reflectedLightVector, eyeDirection);
	specularLight = max(specularLight,0);

	specularLight = pow(specularLight, 32);
	specularLight *= ks;
	if(difuseTest <= 0.001) specularLight = 0;

	vec3 color = texture2D(u_albedoSampler, v_texCoord).xyz;
	vec3 difuseVec = vec3(difuseLight,difuseLight,difuseLight);
	vec3 ambientVec = color * ka;
	vec3 specularVec = vec3(specularLight,specularLight,specularLight);

	//ambientVec *= mix(skyBoxColor.rgb, skyBoxIntensity.rgb, 0.5);
	ambientVec = mix(ambientVec, skyBoxColor, 0.5);
	//ambientVec *= skyBoxColor;

	color *= (ambientVec + difuseVec + specularVec);
	

	//color = skyBoxColor;

	a_outColor = clamp(vec4(color,1), 0, 1);


}