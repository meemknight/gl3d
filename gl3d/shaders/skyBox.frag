#version 330

out vec4 fragColor;

in vec3 v_texCoords;

uniform samplerCube u_skybox;

void main()
{    
    fragColor = texture(u_skybox, v_texCoords);
}