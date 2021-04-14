#version 430
#pragma debug(on)

#extension GL_NV_shadow_samplers_cube : enable

layout(location = 0) out vec3 a_pos;
layout(location = 1) out vec3 a_normal;
layout(location = 2) out vec3 a_outColor;
layout(location = 3) out vec3 a_material;
layout(location = 4) out vec3 a_posViewSpace;
layout(location = 5) out vec3 a_normalViewSpace;

in vec3 v_normals;
in vec3 v_position;	//world space
in vec2 v_texCoord;
in vec3 v_positionViewSpace;

uniform sampler2D u_albedoSampler;
uniform sampler2D u_normalSampler;

uniform sampler2D u_RMASampler;
uniform int u_materialIndex;



struct MaterialStruct
{
	vec4 kd;
	vec4 rma;
	//float kdr; //= 1;
	//float kdg; //= 1;
	//float kdb; //= 1;
	//float roughness;
	//float metallic;
	//float ao; //one means full light
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


//albedo
subroutine vec4 GetAlbedoFunc();

subroutine (GetAlbedoFunc) vec4 sampledAlbedo()
{
	vec4 color = texture2D(u_albedoSampler, v_texCoord).xyzw;
		if(color.w <= 0.1)
			discard;

	color.rgb = pow(color.rgb , vec3(2.2,2.2,2.2)).rgb; //gamma corection
		
	color *= vec4(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b, 1); //(option) multiply texture by kd
		

	return color;
}

subroutine (GetAlbedoFunc) vec4 notSampledAlbedo()
{
	vec4 c = vec4(mat[u_materialIndex].kd.r, mat[u_materialIndex].kd.g, mat[u_materialIndex].kd.b, 1);	

	c.rgb = pow(c.rgb , vec3(2.2,2.2,2.2)).rgb;

	return c;
}

subroutine uniform GetAlbedoFunc u_getAlbedo;


subroutine vec3 GetMaterialMapped();

subroutine (GetMaterialMapped) vec3 materialNone()
{
	return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}

subroutine (GetMaterialMapped) vec3 materialR()
{
	float r = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(r, mat[u_materialIndex].rma.y, mat[u_materialIndex].rma.z);
}

subroutine (GetMaterialMapped) vec3 materialM()
{
	float m = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(mat[u_materialIndex].rma.x, m, mat[u_materialIndex].rma.z);
}

subroutine (GetMaterialMapped) vec3 materialA()
{
	float a = texture2D(u_RMASampler, v_texCoord).r;
	return vec3(mat[u_materialIndex].rma.x, mat[u_materialIndex].rma.y, a);
}

subroutine (GetMaterialMapped) vec3 materialRM()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).rg;
	return vec3(v.x, v.y, mat[u_materialIndex].rma.z);
}

subroutine (GetMaterialMapped) vec3 materialRA()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).rb;
	return vec3(v.x, mat[u_materialIndex].rma.y, v.y);
}

subroutine (GetMaterialMapped) vec3 materialMA()
{
	vec2 v = texture2D(u_RMASampler, v_texCoord).gb;
	return vec3(mat[u_materialIndex].rma.x, v.x, v.y);
}

subroutine (GetMaterialMapped) vec3 materialRMA()
{
	return texture2D(u_RMASampler, v_texCoord).rgb;
}

subroutine uniform GetMaterialMapped u_getMaterialMapped;




void main()
{


	vec3 sampledMaterial = u_getMaterialMapped();
	float roughnessSampled = sampledMaterial.r;
	roughnessSampled = max(0.09,roughnessSampled);
	float metallicSampled = sampledMaterial.g;
	float sampledAo = sampledMaterial.b;


	vec4 color  = u_getAlbedo(); //texture color


	vec3 noMappedNorals = normalize(v_normals);
	vec3 normal = getNormalMapFunc(noMappedNorals);
	//normal = noMappedNorals; //(option) remove normal mapping


	a_pos = v_position;
	a_normal = normalize(normal);
	a_outColor = clamp(color.rgb, 0, 1);
	a_material = vec3(roughnessSampled, metallicSampled, sampledAo);
	a_posViewSpace = v_positionViewSpace;

}