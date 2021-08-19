#version 150 core
//https://www.youtube.com/watch?v=Z9bYzpwVINA&t=1661s
out vec4 a_color;

in vec2 v_texCoords;

uniform sampler2D u_texture;

float luminance(in vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 getTexture(in vec2 offset)
{
	return texture2D(u_texture, v_texCoords + offset).rgb;
}

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
	 
	float minScale = 1.0/max(min(abs(dir.x), abs(dir.y)), dirReduce);
	dir = clamp(vec2(-fxaaSpan), vec2(fxaaSpan), dir * minScale);

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