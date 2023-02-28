#version 150 core

out vec4 a_color;

noperspective in vec2 v_texCoords;

uniform sampler2D u_texture;
uniform sampler2D u_lookup;

int w=8;
int h=8;

vec3 sampleBlueCube(int index, vec2 rg)
{
	ivec2 index2;
	index2.x = index % w;
	index2.y = index / w;

	vec2 cellSize = vec2(1)/vec2(w,h);

	vec2 cellPos = cellSize * rg;

	vec2 finalCellPos = cellPos + cellSize*index2;

	finalCellPos.y = 1-finalCellPos.y;
	return texture(u_lookup, finalCellPos).rgb;
}

void main()
{
	vec3 color = texture(u_texture, v_texCoords).rgb;

	float indexFloat = color.b * w * h;
	int index1 = int(floor(color.b * w * h));
	int index2 = int(ceil(color.b * w * h));

	float decimal = indexFloat - index1;

	a_color.rgb = mix(sampleBlueCube(index1, color.rg), sampleBlueCube(index2, color.rg), vec3(decimal));
	a_color.a = 1;

}