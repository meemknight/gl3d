#version 330

out layout(location = 0) vec4 outColor;

in vec3 v_normals;
in vec3 v_position;

uniform vec3 u_lightPosition;

void main()
{


	vec3 lightDirection = u_lightPosition - v_position;
	lightDirection = normalize(lightDirection);


	float difuseLight = dot(lightDirection, v_normals);

	difuseLight = max(difuseLight, 0);
	

	outColor = vec4(0.4,0.4,0.4,1);
	outColor *= difuseLight;

	outColor += 0.1;

	outColor = min(outColor, 1);
	outColor = max(outColor, 0);

}