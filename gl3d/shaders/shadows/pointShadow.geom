#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_shadowMatrices[6];

out vec4 v_fragPos; // FragPos from GS (output per emitvertex)
out vec2 v_finalTexCoord;


in vec2 v_texCoord[3];

void main()
{
	for(int face = 0; face < 6; ++face)
	{
		gl_Layer = face; // built-in variable that specifies to which face we render.
		for(int i = 0; i < 3; ++i) // for each triangle vertex
		{
			v_fragPos = gl_in[i].gl_Position;
			v_finalTexCoord = v_texCoord[i];
			gl_Position = u_shadowMatrices[face] * v_fragPos;
			EmitVertex();
		}    
		EndPrimitive();
	}
}  

