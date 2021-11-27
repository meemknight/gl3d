#version 150
#pragma debug(on)

uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
uniform vec3 u_lightPos;
uniform float u_farPlane;

in vec4 v_fragPos;
in vec2 v_finalTexCoord;

void main()
{

	if(u_hasTexture != 0)
		{
			float albedo = texture2D(u_albedoSampler, v_finalTexCoord).a;
				if(albedo*255 < 1)
				discard;
		}

	// get distance between fragment and light source
	float lightDistance = length(v_fragPos.xyz - u_lightPos);
	
	// map to [0;1] range by dividing by far_plane
	lightDistance = lightDistance / u_farPlane;
	
	// write this as modified depth
	gl_FragDepth = lightDistance;

	
}