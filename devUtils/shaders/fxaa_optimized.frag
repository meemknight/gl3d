#version 150 core
//https://www.youtube.com/watch?v=Z9bYzpwVINA&t=1661s
out vec4 a_color;

in vec2 v_texCoords;

uniform sampler2D u_texture;

float luminance(in vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

float lumaSqr(in vec3 color)
{
	return sqrt(luminance(color));
}

vec3 getTexture(in vec2 offset)
{
	return texture2D(u_texture, v_texCoords + offset).rgb;
}

float quality(int i)
{
	const float r[7] = float[7](1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);

	if(i < 5)
	{
		return 1;
	}else if(i >= 12)
	{
		return 8;
	}else return r[i-5];
}

//http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html
void main()
{

	float edgeMinTreshold = 0.0312;
	float edgeDarkTreshold = 0.125;
	int ITERATIONS = 12;
	float SUBPIXEL_QUALITY = 0.75;

	vec3 colorCenter = getTexture(vec2(0,0)).rgb;
	
	// Luma at the current fragment
	float lumaCenter = lumaSqr(colorCenter);

	// Luma at the four direct neighbours of the current fragment.
	float lumaDown = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(0,-1)).rgb);
	float lumaUp = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(0,1)).rgb);
	float lumaLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,0)).rgb);
	float lumaRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,0)).rgb);
	
	// Find the maximum and minimum luma around the current fragment.
	float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
	float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));
	
	// Compute the delta.
	float lumaRange = lumaMax - lumaMin;
	
	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
	if(lumaRange < max(edgeMinTreshold,lumaMax*edgeDarkTreshold))
	{
		a_color = vec4(colorCenter, 1);
		return;
	}
	
	// Query the 4 remaining corners lumas.
	float lumaDownLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,-1)).rgb);
	float lumaUpRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,1)).rgb);
	float lumaUpLeft = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(-1,1)).rgb);
	float lumaDownRight = lumaSqr(textureOffset(u_texture,v_texCoords,ivec2(1,-1)).rgb);
	
	// Combine the four edges lumas (using intermediary variables for future computations with the same values).
	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;
	
	// Same for corners
	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;
	
	// Compute an estimation of the gradient along the horizontal and vertical axis.
	float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);
	
	// Is the local edge horizontal or vertical ?
	bool isHorizontal = (edgeHorizontal >= edgeVertical);


	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;
	// Compute gradients in this direction.
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;
	
	// Which direction is the steepest ?
	bool is1Steepest = abs(gradient1) >= abs(gradient2);
	
	// Gradient in the corresponding direction, normalized.
	float gradientScaled = 0.25*max(abs(gradient1),abs(gradient2));

	vec2 inverseScreenSize = 1.f/textureSize(u_texture, 0);

	// Choose the step size (one pixel) according to the edge direction.
	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
	
	// Average luma in the correct direction.
	float lumaLocalAverage = 0.0;
	
	if(is1Steepest)
	{
		// Switch the direction
		stepLength = - stepLength;
		lumaLocalAverage = 0.5*(luma1 + lumaCenter);
	} 
	else
	{
		lumaLocalAverage = 0.5*(luma2 + lumaCenter);
	}
	
	// Shift UV in the correct direction by half a pixel.
	vec2 currentUv = v_texCoords;
	if(isHorizontal)
	{
		currentUv.y += stepLength * 0.5;
	} else 
	{
		currentUv.x += stepLength * 0.5;
	}

	// Compute offset (for each iteration step) in the right direction.
	vec2 offset = isHorizontal ? vec2(inverseScreenSize.x,0.0) : vec2(0.0,inverseScreenSize.y);
	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
	vec2 uv1 = currentUv - offset;
	vec2 uv2 = currentUv + offset;
	
	// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
	float lumaEnd1 = lumaSqr(texture(u_texture,uv1).rgb);
	float lumaEnd2 = lumaSqr(texture(u_texture,uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;
	
	// If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;
	
	// If the side is not reached, we continue to explore in this direction.
	if(!reached1){
		uv1 -= offset;
	}
	if(!reached2){
		uv2 += offset;
	}   


	// If both sides have not been reached, continue to explore.
	if(!reachedBoth)
	{

		for(int i = 2; i < ITERATIONS; i++){
			// If needed, read luma in 1st direction, compute delta.
			if(!reached1){
				lumaEnd1 = lumaSqr(texture(u_texture, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			// If needed, read luma in opposite direction, compute delta.
			if(!reached2){
				lumaEnd2 = lumaSqr(texture(u_texture, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable quality.
			if(!reached1)
			{
				uv1 -= offset * quality(i);
			}
			if(!reached2)
			{
				uv2 += offset * quality(i);
			}

			// If both sides have been reached, stop the exploration.
			if(reachedBoth){ break;}
		}
	}

	// Compute the distances to each extremity of the edge.
	float distance1 = isHorizontal ? (v_texCoords.x - uv1.x) : (v_texCoords.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - v_texCoords.x) : (uv2.y - v_texCoords.y);
	
	// In which direction is the extremity of the edge closer ?
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);
	
	// Length of the edge.
	float edgeThickness = (distance1 + distance2);
	
	// UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;

	// Is the luma at center smaller than the local average ?
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	
	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	// (in the direction of the closer side of the edge.)
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
	
	// If the luma variation is incorrect, do not offset.
	float finalOffset = correctVariation ? pixelOffset : 0.0;


	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	// Compute a sub-pixel offset based on this delta.
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;
	
	// Pick the biggest of the two offsets.
	finalOffset = max(finalOffset,subPixelOffsetFinal);


	// Compute the final UV coordinates.
	vec2 finalUv = v_texCoords;
	if(isHorizontal){
		finalUv.y += finalOffset * stepLength;
	} else {
		finalUv.x += finalOffset * stepLength;
	}
	
	// Read the color at the new UV coordinates, and use it.
	vec3 finalColor = texture(u_texture, finalUv).rgb;
	a_color = vec4(finalColor, 1);

}


/*
//https://www.youtube.com/watch?v=Z9bYzpwVINA&t=1661s
void main()
{
	float fxaaSpan = 2.0;
	float fxaaReduceMin = 0.001;
	float fxaaReduceMul = 0.1;

	vec2 tSize = 1.f/textureSize(u_texture, 0).xy;

	vec3 tL		= getTexture(vec2(-tSize.x,-tSize.y));
	vec3 tR		= getTexture(vec2(tSize.x,-tSize.y));
	vec3 mid	= getTexture(vec2(0,0));
	vec3 bL		= getTexture(vec2(-tSize.x,tSize.y));
	vec3 bR		= getTexture(vec2(tSize.x,tSize.y));

	float lumaTL = luminance(tL);	
	float lumaTR = luminance(tR);	
	float lumaMid = luminance(mid);
	float lumaBL = luminance(bL);
	float lumaBR = luminance(bR);

	float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * 0.25f * fxaaReduceMul, fxaaReduceMin);

	vec2 dir;
	dir.x = -((lumaTL + lumaTR)-(lumaBL + lumaBR));
	dir.y = (lumaTL + lumaBL)-(lumaTR + lumaBR);
	 
	float minScale = 1.0/min(abs(dir.x), abs(dir.y)+dirReduce);
	dir = clamp(vec2(-fxaaSpan), vec2(fxaaSpan), dir * minScale);

	if(abs(dir).x < 0.1 && abs(dir).y < 0.1)
	{
		a_color = vec4(mid,1);
	}else
	{
		dir *= tSize;
		
		vec3 rezult1 = 0.5 * (
			getTexture(dir * vec2(1.f/3.f - 0.5f))+
			getTexture(dir * vec2(2.f/3.f - 0.5f))
		);

		vec3 rezult2 = rezult1*0.5 + 0.25 * (
			getTexture(dir * vec2(-0.5f))+
			getTexture(dir * vec2(0.5f))
		);

		float lumaRez2 = luminance(rezult2);
		
		float lumaMin = min(min(min(min(lumaTL,lumaTR),lumaMid), lumaBL), lumaBR);
		float lumaMax = max(max(max(max(lumaTL,lumaTR),lumaMid), lumaBL), lumaBR);

		if(lumaRez2 < lumaMin || lumaRez2 > lumaMax)
		{
			a_color = vec4(rezult1,1);
		}else
		{
			a_color = vec4(rezult2,1);
		}
	}


}
*/