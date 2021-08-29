#version 150
out vec3 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform int u_mip;
void main ()
{
  vec2 tmpvar_1;
  tmpvar_1 = (1.0/(vec2(textureSize (u_texture, u_mip))));
  vec2 tmpvar_2;
  tmpvar_2 = (tmpvar_1 * 2.0);
  float tmpvar_3;
  tmpvar_3 = float(u_mip);
  vec4 tmpvar_4;
  tmpvar_4 = textureLod (u_texture, (v_texCoords + (tmpvar_2 * vec2(0.0, 1.0))), tmpvar_3);
  vec4 tmpvar_5;
  tmpvar_5 = textureLod (u_texture, (v_texCoords + (tmpvar_2 * vec2(-1.0, 0.0))), tmpvar_3);
  vec4 tmpvar_6;
  tmpvar_6 = textureLod (u_texture, v_texCoords, tmpvar_3);
  vec4 tmpvar_7;
  tmpvar_7 = textureLod (u_texture, (v_texCoords + (tmpvar_2 * vec2(1.0, 0.0))), tmpvar_3);
  vec4 tmpvar_8;
  tmpvar_8 = textureLod (u_texture, (v_texCoords + (tmpvar_2 * vec2(0.0, -1.0))), tmpvar_3);
  a_color = (((
    ((0.125 * ((textureLod (u_texture, 
      (v_texCoords + (tmpvar_1 * vec2(-1.0, 1.0)))
    , tmpvar_3).xyz + textureLod (u_texture, 
      (v_texCoords + tmpvar_1)
    , tmpvar_3).xyz) + (textureLod (u_texture, 
      (v_texCoords + (tmpvar_1 * vec2(1.0, -1.0)))
    , tmpvar_3).xyz + textureLod (u_texture, 
      (v_texCoords - tmpvar_1)
    , tmpvar_3).xyz))) + (0.03125 * ((textureLod (u_texture, 
      (v_texCoords + (tmpvar_2 * vec2(-1.0, 1.0)))
    , tmpvar_3).xyz + tmpvar_4.xyz) + (tmpvar_5.xyz + tmpvar_6.xyz))))
   + 
    (0.03125 * ((textureLod (u_texture, (v_texCoords + tmpvar_2), tmpvar_3).xyz + tmpvar_4.xyz) + (tmpvar_7.xyz + tmpvar_6.xyz)))
  ) + (0.03125 * 
    ((textureLod (u_texture, (v_texCoords - tmpvar_2), tmpvar_3).xyz + tmpvar_8.xyz) + (tmpvar_5.xyz + tmpvar_6.xyz))
  )) + (0.03125 * (
    (textureLod (u_texture, (v_texCoords + (tmpvar_2 * vec2(1.0, -1.0))), tmpvar_3).xyz + tmpvar_8.xyz)
   + 
    (tmpvar_7.xyz + tmpvar_6.xyz)
  )));
}

