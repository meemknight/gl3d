#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec3 v_normals[];

uniform float u_size = 0.5;

uniform mat4 u_projection; //projection matrix

void emitNormal(int index)
{
	gl_Position = u_projection * gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = u_projection * (gl_in[index].gl_Position + vec4(v_normals[index],0) * u_size);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	
	emitNormal(0);
	emitNormal(1);
	emitNormal(2);

}