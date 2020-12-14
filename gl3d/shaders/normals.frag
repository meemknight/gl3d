#version 330

out layout(location = 0) vec4 a_outColor;

in vec3 v_normals;
in vec3 v_position;	//world space

uniform vec3 u_lightPosition;
uniform mat4 u_normalTransform;

void main()
{

	

	vec3 lightDirection = u_lightPosition - v_position;
	lightDirection = normalize(lightDirection);


	float difuseLight = dot(lightDirection, v_normals);

	difuseLight = max(difuseLight, 0);
	

	a_outColor = vec4(0.4,0.4,0.4,1);
	a_outColor *= difuseLight;

	a_outColor += 0.1;

	a_outColor = clamp(a_outColor, 0, 1);


}