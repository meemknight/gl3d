#version 430
#pragma debug(on)

layout(location = 0) in vec3 a_positions;
layout(location = 1) in vec3 a_normals;
layout(location = 2) in vec2 a_texCoord;
layout(location = 3) in ivec4 a_jointsId;
layout(location = 4) in vec4 a_weights;

uniform mat4 u_transform; //full model view projection
uniform mat4 u_modelTransform; //just model to world
uniform mat4 u_motelViewTransform; //model to world to view

out vec3 v_normals;
out vec3 v_position;
out vec2 v_texCoord;
out vec3 v_positionViewSpace;

readonly restrict layout(std140) buffer u_jointTransforms
{
	mat4 jointTransforms[];
};
uniform int u_hasAnimations;


void main()
{
	vec4 totalLocalPos = vec4(0.f);
	vec4 totalNorm = vec4(0.f);


	if(false)
	{
		for(int i=0; i<4; i++)
		{
			mat4 jointTransform = jointTransforms[a_jointsId[i]];
			vec4 posePosition = jointTransform * vec4(a_positions, 1);
			totalLocalPos += posePosition * a_weights[i];
			
			vec3 worldNormal = mat3(transpose(inverse(mat3(jointTransform)))) * a_normals.xyz;// jointTransform * vec4(a_normals, 1);
			totalNorm.xyz += worldNormal.xyz * a_weights[i];
		}

		totalNorm.xyz = normalize(totalNorm.xyz);
	}else
	{
		totalLocalPos = vec4(a_positions, 1.f);
		totalNorm = vec4(a_normals, 1);
	}


	v_positionViewSpace = vec3(u_motelViewTransform * totalLocalPos);
	
	gl_Position = u_transform * totalLocalPos;

	v_position = (u_modelTransform * totalLocalPos).xyz;

	//v_normals = (u_modelTransform * vec4(a_normals,0)).xyz; //uniform scale
	v_normals = mat3(transpose(inverse(mat3(u_modelTransform)))) * totalNorm.xyz;  //non uniform scale

	v_normals = normalize(v_normals);

	v_texCoord = a_texCoord;
	
}