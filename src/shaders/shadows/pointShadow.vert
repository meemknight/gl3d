#version 430
#pragma debug(on)

layout(location = 0) in vec3 a_positions;
layout(location = 1) in vec3 a_normals; //todo comment out
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in ivec4 a_jointsId;
layout(location = 4) in vec4 a_weights;

uniform mat4 u_transform; //full model view projection or just model for point shadows

readonly restrict layout(std140) buffer u_jointTransforms
{
	mat4 jointTransforms[];
};
uniform int u_hasAnimations;

out vec2 v_texCoord;

void main()
{
	vec4 totalLocalPos = vec4(0.f);

	if(u_hasAnimations != 0)
	//if(false)
	{
		for(int i=0; i<4; i++)
		{
			if(a_jointsId[i] < 0){break;}

			mat4 jointTransform = jointTransforms[a_jointsId[i]];
			vec4 posePosition = jointTransform * vec4(a_positions, 1);
			totalLocalPos += posePosition * a_weights[i];
			
		}

	}else
	{
		totalLocalPos = vec4(a_positions, 1.f);
	}


	gl_Position = u_transform * totalLocalPos;
	v_texCoord = a_texCoord;
	
} 