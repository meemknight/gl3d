#version 150 core
//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//
//--//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
//moddified

uniform vec3 u_lightPos;
uniform vec3 u_color1;
uniform vec3 u_color2;
uniform vec3 u_groundColor;
uniform float u_g;

in vec3 v_localPos;
out vec3 fragColor;

void main (void)
{

vec3 v3CameraPos = vec3(0,0,0);		// The camera's current position
vec3 v3InvWavelength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels

float fCameraHeight = 0;	// The camera's current height
float fCameraHeight2 = fCameraHeight * fCameraHeight;	// fCameraHeight^2
float fOuterRadius;		// The outer (atmosphere) radius
float fOuterRadius2 = fOuterRadius * fOuterRadius;	// fOuterRadius^2
float fInnerRadius;		// The inner (planetary) radius
float fInnerRadius2 = fInnerRadius * fInnerRadius;	// fInnerRadius^2
float fKrESun;			// Kr * ESun
float fKmESun;			// Km * ESun
float fKr4PI;			// Kr * 4 * PI
float fKm4PI;			// Km * 4 * PI
float fScale;			// 1 / (fOuterRadius - fInnerRadius)
float fScaleDepth;		// The scale depth (i.e. the altitude at which the atmosphere's average density is found)
float fScaleOverScaleDepth;	// fScale / fScaleDepth

	vec3 skyColor = u_color1;
	vec3 sunColor = u_color2;

	vec3 localPos = normalize(v_localPos);
	vec3 lightPos = normalize(u_lightPos);

	vec3 upVector = vec3(0,1,0);
	float fCosEarth = max(dot(localPos, upVector),0);
	float foneMinusCosEarth = 1-fCosEarth;
	//float fCosSunEarth = 1-max(dot(lightPos, upVector), 0);
	float fCosSunEarth = 1-abs(dot(lightPos, upVector));

	float g2 = u_g * u_g;
	
	float fCosSun = dot(lightPos, localPos);
	float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCosSun*fCosSun) / pow(1.0 + g2 - 2.0*u_g*fCosSun, 1.5);
	float horizonIntensity = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCosSunEarth*fCosSunEarth) / pow(1.0 + g2 - 2.0*u_g*fCosSunEarth, 1.5);

	vec3 computedSkyColor = skyColor
							+ fMiePhase * sunColor 
							+ (fCosSunEarth)*sunColor* 4 * pow(foneMinusCosEarth, 12)
							+ pow(foneMinusCosEarth, 16) * sunColor * 4
							;

	//u_groundColor = vec3(0.2,0.2,0.2);

	vec3 computedGroundColor = vec3(0.1,0.9,0.1);// + pow(foneMinusCosEarth, 16) * sunColor;

	if(fCosEarth < 0.01)
	{
		fragColor.rgb = computedGroundColor;	
	}else
	{
		fragColor.rgb =  computedSkyColor;
	}


	//fragColor.rgb =  firstColor + vec3(fCos);

}
