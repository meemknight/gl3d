#version 150 core

//layout (location = 0) out vec4 a_color;
out vec4 a_color;

noperspective in vec2 v_texCoords;

uniform sampler2D u_colorTexture;
uniform sampler2D u_bloomTexture;
uniform sampler2D u_bloomNotBluredTexture;

uniform float u_bloomIntensity;
uniform float u_exposure;

uniform int u_useSSAO;
uniform float u_ssaoExponent;
uniform sampler2D u_ssao;

//https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
/*
=================================================================================================

  Baking Lab
  by MJP and David Neubelt
  http://mynameismjp.wordpress.com/

  All code licensed under the MIT license

=================================================================================================
 The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
 credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)
*/

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3x3 ACESInputMat = mat3x3
(
	0.59719, 0.35458, 0.04823,
	0.07600, 0.90834, 0.01566,
	0.02840, 0.13383, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3x3 ACESOutputMat = mat3x3
(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
	vec3 a = v * (v + 0.0245786f) - 0.000090537f;
	vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

vec3 ACESFitted(vec3 color)
{
	color = transpose(ACESInputMat) * color;
	// Apply RRT and ODT
	color = RRTAndODTFit(color);
	color = transpose(ACESOutputMat) * color;
	color = clamp(color, 0, 1);
	return color;
}

vec3 ACESFilmSimple(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0, 1);

}


//https://www.shadertoy.com/view/4tXcWr
//srgb////////////////////////////////////////////////////////////////
vec3 fromLinearSRGB(vec3 linearRGB)
{
	bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linearRGB * vec3(12.92);

	return mix(higher, lower, cutoff);
}
//////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
//ZCAM
//https://www.shadertoy.com/view/dlGBDD
//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
/////////////////////////////////////////////////////////////////////////////////////////


// eotf_pq parameters
const float Zcam_Lp = 10000.0;
const float Zcam_m1 = 2610.0 / 16384.0;
const float Zcam_m2 = 1.7 * 2523.0 / 32.0;
const float Zcam_c1 = 107.0 / 128.0;
const float Zcam_c2 = 2413.0 / 128.0;
const float Zcam_c3 = 2392.0 / 128.0;

vec3 eotf_pq(vec3 x)
{
	x = sign(x) * pow(abs(x), vec3(1.0 / Zcam_m2));
	x = sign(x) * pow((abs(x) - Zcam_c1) / (Zcam_c2 - Zcam_c3 * abs(x)), vec3(1.0 / Zcam_m1)) * Zcam_Lp;
	return x;
}

vec3 eotf_pq_inverse(vec3 x)
{
	x /= Zcam_Lp;
	x = sign(x) * pow(abs(x), vec3(Zcam_m1));
	x = sign(x) * pow((Zcam_c1 + Zcam_c2 * abs(x)) / (1.0 + Zcam_c3 * abs(x)), vec3(Zcam_m2));
	return x;
}

// XYZ <-> ICh parameters
const float Zcam_W = 140.0;
const float Zcam_b = 1.15;
const float Zcam_g = 0.66;

vec3 XYZ_to_ICh(vec3 XYZ)
{
	XYZ *= Zcam_W;
	XYZ.xy = vec2(Zcam_b, Zcam_g) * XYZ.xy - (vec2(Zcam_b, Zcam_g) - 1.0) * XYZ.zx;
	
	const mat3 XYZ_to_LMS = transpose(mat3(
		 0.41479,   0.579999, 0.014648,
		-0.20151,   1.12065,  0.0531008,
		-0.0166008, 0.2648,   0.66848));
	
	vec3 LMS = XYZ_to_LMS * XYZ;
	LMS = eotf_pq_inverse(LMS);
	
	const mat3 LMS_to_Iab = transpose(mat3(
		0.0,       1.0,      0.0,
		3.524,    -4.06671,  0.542708,
		0.199076,  1.0968,  -1.29588));
	
	vec3 Iab = LMS_to_Iab * LMS;
	
	float I = eotf_pq(vec3(Iab.x)).x / Zcam_W;
	float C = length(Iab.yz);
	float h = atan(Iab.z, Iab.y);
	return vec3(I, C, h);
}

vec3 ICh_to_XYZ(vec3 ICh)
{
	vec3 Iab;
	Iab.x = eotf_pq_inverse(vec3(ICh.x * Zcam_W)).x;
	Iab.y = ICh.y * cos(ICh.z);
	Iab.z = ICh.y * sin(ICh.z);
	
	const mat3 Iab_to_LMS = transpose(mat3(
		1.0, 0.2772,  0.1161,
		1.0, 0.0,     0.0,
		1.0, 0.0426, -0.7538));
	
	vec3 LMS = Iab_to_LMS * Iab;
	LMS = eotf_pq(LMS);
	
	const mat3 LMS_to_XYZ = transpose(mat3(
		 1.92423, -1.00479,  0.03765,
		 0.35032,  0.72648, -0.06538,
		-0.09098, -0.31273,  1.52277));
	
	vec3 XYZ = LMS_to_XYZ * LMS;
	XYZ.x = (XYZ.x + (Zcam_b - 1.0) * XYZ.z) / Zcam_b;
	XYZ.y = (XYZ.y + (Zcam_g - 1.0) * XYZ.x) / Zcam_g;
	return XYZ / Zcam_W;
}

const mat3 XYZ_to_sRGB = transpose(mat3(
	 3.2404542, -1.5371385, -0.4985314,
	-0.9692660,  1.8760108,  0.0415560,
	 0.0556434, -0.2040259,  1.0572252));

const mat3 sRGB_to_XYZ = transpose(mat3(
	0.4124564, 0.3575761, 0.1804375,
	0.2126729, 0.7151522, 0.0721750,
	0.0193339, 0.1191920, 0.9503041));

bool in_sRGB_gamut(vec3 ICh)
{
	vec3 sRGB = XYZ_to_sRGB * ICh_to_XYZ(ICh);
	return all(greaterThanEqual(sRGB, vec3(0.0))) && all(lessThanEqual(sRGB, vec3(1.0)));
}

vec3 Zcam_tonemap(vec3 sRGB)
{	
	vec3 ICh = XYZ_to_ICh(sRGB_to_XYZ * sRGB);
	
	const float s0 = 0.71;
	const float s1 = 1.04;
	const float p = 1.40;
	const float t0 = 0.01;
	float n = s1 * pow(ICh.x / (ICh.x + s0), p);
	ICh.x = clamp(n * n / (n + t0), 0.0, 1.0);
	
	if (!in_sRGB_gamut(ICh))
	{
		float C = ICh.y;
		ICh.y -= 0.5 * C;
		
		for (float i = 0.25; i >= 1.0 / 256.0; i *= 0.5)
		{
			ICh.y += (in_sRGB_gamut(ICh) ? i : -i) * C;
		}
	}
	
	return XYZ_to_sRGB * ICh_to_XYZ(ICh);
}


/////////////////////////////////////////////////////////////////////////////////////////
//^ZCAM^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/////////////////////////////////////////////////////////////////////////////////////////


uniform int u_tonemapper = 0;

void main()
{
	vec4 color = texture(u_colorTexture, v_texCoords).rgba;

	//a_color = color.aaa;
	//return;

	float ssaof = 1;
	if(u_useSSAO != 0)
	{
		ssaof = texture(u_ssao, v_texCoords).r;	
		ssaof = pow(ssaof, u_ssaoExponent);
	}else
	{
		ssaof = 1;
	}
	
	vec3 bloom = texture(u_bloomTexture, v_texCoords).rgb;
	vec3 bloomNotBlurred = texture(u_bloomNotBluredTexture, v_texCoords).rgb;


	//if(color.a < 0.5 && bloom.r == 0 && bloom.g == 0 && bloom.b == 0){discard;} //this is optional
	//if(color.a < 0.5){discard;}


	//a_color.rgb = (bloom * u_bloomIntensity) + (bloomNotBlurred + color.rgb) * ssaof;
	a_color.rgb = (bloom * u_bloomIntensity) + (color.rgb) * ssaof;
	//a_color = bloom;
	
	//this code is duplicated in filter !!!!!!!!!!!!!!!!!!!!!!!!!!
	if(u_tonemapper == 0)
	{
		a_color.rgb = vec3(1.0) - exp(-a_color.rgb  * u_exposure);
	}
	else if(u_tonemapper == 1)
	{
		a_color.rgb = ACESFitted(a_color.rgb * u_exposure);
		//a_color.rgb = ACESFilmSimple(a_color.rgb * u_exposure);
	}else if(u_tonemapper == 2)
	{
		a_color.rgb = Zcam_tonemap(a_color.rgb * u_exposure);
	}
	

	//gamma correction
	//a_color.rgb = pow(a_color.rgb, vec3(1.0/2.2));
	a_color.rgb = clamp(fromLinearSRGB(a_color.rgb), 0.f, 1.f);

	//a_color.rgb = vec3(ssaof);

	a_color.a =  color.a;
}