#version 330

out layout(location = 0) vec4 a_outColor;

in vec3 v_normals;
in vec3 v_position;	//world space

uniform vec3 u_lightPosition;
uniform vec3 u_eyePosition;

void main()
{
	float ka = 0.1;
	float kd = 0.3;
	float ks = 0.6;

	vec3 lightDirection = u_lightPosition - v_position;
	lightDirection = normalize(lightDirection);

	float difuseLight = dot(v_normals, lightDirection);
	difuseLight = max(difuseLight, 0);
	difuseLight *= kd;

	vec3 eyeDirection = u_eyePosition - v_position;
	eyeDirection = normalize(eyeDirection);
	
	//todo fix
	vec3 reflectedLightVector = reflect(-lightDirection, v_normals);
	float specularLight = dot(reflectedLightVector, eyeDirection);
	specularLight = max(specularLight,0);

	specularLight = pow(specularLight, 32);
	specularLight *= ks;
	if(difuseLight == 0) specularLight = 0;

	vec3 color = vec3(1,1,1);
	vec3 difuseVec = vec3(difuseLight,difuseLight,difuseLight);
	vec3 ambientVec = vec3(ka,ka,ka);

	color *= ambientVec + difuseVec + specularLight;


	a_outColor = clamp(vec4(color,1), 0, 1);


}