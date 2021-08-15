#version 330 core
//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//
//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
//moddified

uniform vec3 u_lightPos;
//uniform float u_g;
//uniform float u_g2;

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



	vec3 firstColor;
	vec3 secondColor;

	vec3 localPos = normalize(v_localPos);
	vec3 lightPos = normalize(u_lightPos);

	float u_g = 0.76;
	float u_g2 = u_g * u_g;
	
	float fCos = dot(lightPos, localPos);
	float fMiePhase = 1.5 * ((1.0 - u_g2) / (2.0 + u_g2)) * (1.0 + fCos*fCos) / pow(1.0 + u_g2 - 2.0*u_g*fCos, 1.5);

	fragColor.rgb =  firstColor + fMiePhase * secondColor;

}
