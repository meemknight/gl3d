#version 150
out vec3 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;
void main ()
{
  vec2 tmpvar_1;
  tmpvar_1 = (1.0/(vec2(textureSize (u_texture, u_mip))));
  float tmpvar_2;
  tmpvar_2 = float(u_mip);
  a_color = textureLod (u_texture, (v_texCoords + (tmpvar_1 * vec2(-1.0, 1.0))), tmpvar_2).xyz;
  a_color = (a_color + (textureLod (u_texture, (v_texCoords + 
    (tmpvar_1 * vec2(0.0, 1.0))
  ), tmpvar_2).xyz * 2.0));
  a_color = (a_color + textureLod (u_texture, (v_texCoords + tmpvar_1), tmpvar_2).xyz);
  a_color = (a_color + (textureLod (u_texture, (v_texCoords + 
    (tmpvar_1 * vec2(-1.0, 0.0))
  ), tmpvar_2).xyz * 2.0));
  a_color = (a_color + (textureLod (u_texture, v_texCoords, tmpvar_2).xyz * 4.0));
  a_color = (a_color + (textureLod (u_texture, (v_texCoords + 
    (tmpvar_1 * vec2(1.0, 0.0))
  ), tmpvar_2).xyz * 2.0));
  a_color = (a_color + textureLod (u_texture, (v_texCoords - tmpvar_1), tmpvar_2).xyz);
  a_color = (a_color + (textureLod (u_texture, (v_texCoords + 
    (tmpvar_1 * vec2(0.0, -1.0))
  ), tmpvar_2).xyz * 2.0));
  a_color = (a_color + textureLod (u_texture, (v_texCoords + (tmpvar_1 * vec2(1.0, -1.0))), tmpvar_2).xyz);
  a_color = (a_color / 16.0);
}

