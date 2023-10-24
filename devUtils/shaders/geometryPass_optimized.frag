#version 430
#pragma debug(on)

//#extension GL_NV_shadow_samplers_cube : enable
#extension GL_ARB_bindless_texture: require

layout(location = 0) out ivec3 a_normal;
layout(location = 1) out ivec4 a_textureDerivates;
layout(location = 2) out vec3 a_posViewSpace;
layout(location = 3) out int a_materialIndex;
layout(location = 4) out vec4 a_textureUV;

in vec3 v_normals;
in vec2 v_texCoord;
in vec3 v_positionViewSpace;

uniform sampler2D u_normalSampler;

//todo remove uniform if possible
//todo add another uniform to do this computation on the cpu if(albedoSampler.x == 0 && albedoSampler.y == 0)
uniform int u_materialIndex;


struct MaterialStruct
{
	vec4 kd;
	vec4 rma; //last component emmisive
	
	//float kdr; //= 1;
	//float kdg; //= 1;
	//float kdb; //= 1;
	//float roughness;
	//float metallic;
	//float ao; //one means full light

	uvec4 firstBIndlessSamplers;  // xy albedoSampler,  zw rmaSampler
	uvec2 secondBIndlessSamplers; // xy emmissiveSampler
	int rmaLoaded;
	int notUsed;

};

readonly layout(std140) buffer u_material
{
	MaterialStruct mat[];
};


float PI = 3.14159265359;

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


subroutine vec3 GetNormalMapFunc(vec3);

subroutine (GetNormalMapFunc) vec3 normalMapped(vec3 v)
{
	vec3 normal = texture2D(u_normalSampler, v_texCoord).rgb;
	normal = normalize(2*normal - 1.f);
	mat3 rotMat = NormalToRotation(v);
	normal = rotMat * normal;
	normal = normalize(normal);
	return normal;
}

subroutine (GetNormalMapFunc) vec3 noNormalMapped(vec3 v)
{
	return v;
}

subroutine uniform GetNormalMapFunc getNormalMapFunc;


int fromFloat2TouShort(float a)
{
	//[-2 2] -> [0 4]
	a += 2.f;

	//[0 4] -> [0 1]
	a /= 4.f;

	//[0 1] -> [0 65536]
	a *= 65536;

	return int(a);
}


ivec3 fromFloatTouShort(vec3 a)
{
	//[-1 1] -> [0 2]
	a += 1.f;

	//[0 2] -> [0 1]
	a /= 2.f;

	//[0 1] -> [0 65536]
	a *= 65536;

	return ivec3(a);
}



void main()
{

	//vec4 color  = u_getAlbedo(); //texture color
	//if(color.a < 0.1)discard;

	uvec2 albedoSampler = mat[u_materialIndex].firstBIndlessSamplers.xy;
		if(albedoSampler.x == 0 && albedoSampler.y == 0)
		{
			//no albedo data
		}else
		{
			float alphaData = texture2D(sampler2D(albedoSampler), v_texCoord).a;
			if(alphaData*255 < 1)
				discard;
		}


	vec3 noMappedNorals = normalize(v_normals);
	vec3 normal = getNormalMapFunc(noMappedNorals);
	//normal = noMappedNorals; //(option) remove normal mapping


	normal = normalize(normal);
	a_normal = fromFloatTouShort(normal);


	a_posViewSpace = v_positionViewSpace;
	a_materialIndex = u_materialIndex+1;
	a_textureUV.xy = v_texCoord.xy;
	a_textureDerivates.x = fromFloat2TouShort(dFdx(v_texCoord.x));
	a_textureDerivates.y = fromFloat2TouShort(dFdy(v_texCoord.x));
	a_textureDerivates.z = fromFloat2TouShort(dFdx(v_texCoord.y));
	a_textureDerivates.w = fromFloat2TouShort(dFdy(v_texCoord.y));
	//a_emmisive = a_outColor.rgb * mat[u_materialIndex].rma.a;

	//a_emmisive = texture2D(u_emissiveTexture, v_texCoord).rgb;

}