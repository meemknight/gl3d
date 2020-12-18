#version 330

out layout(location = 0) vec4 a_outColor;

in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;

uniform vec3 u_lightPosition;
uniform vec3 u_eyePosition;

uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;

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
	float ka = 0.15;
	float kd = 0.45;
	float ks = 0.9;


	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);

	mat3 rotMat = NormalToRotation(v_normals);
	normal = rotMat * normal;


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
	vec3 ambientVec = vec3(ka,ka,ka);

	color *= ambientVec + difuseVec + specularLight;


	a_outColor = clamp(vec4(color,1), 0, 1);


}