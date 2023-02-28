#version 150
out vec4 a_color;
noperspective in vec2 v_texCoords;
uniform sampler2D u_texture;
uniform sampler2D u_lookup;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1 = texture (u_texture, v_texCoords);
  int tmpvar_2;
  tmpvar_2 = int(floor((64.0 * tmpvar_1.z)));
  int tmpvar_3;
  tmpvar_3 = int(ceil((64.0 * tmpvar_1.z)));
  vec2 finalCellPos_4;
  ivec2 index2_5;
  index2_5.x = (int(mod (tmpvar_2, 8)));
  index2_5.y = (tmpvar_2 / 8);
  vec2 tmpvar_6;
  tmpvar_6 = ((vec2(0.125, 0.125) * tmpvar_1.xy) + (vec2(0.125, 0.125) * vec2(index2_5)));
  finalCellPos_4.x = tmpvar_6.x;
  finalCellPos_4.y = (1.0 - tmpvar_6.y);
  vec2 finalCellPos_7;
  ivec2 index2_8;
  index2_8.x = (int(mod (tmpvar_3, 8)));
  index2_8.y = (tmpvar_3 / 8);
  vec2 tmpvar_9;
  tmpvar_9 = ((vec2(0.125, 0.125) * tmpvar_1.xy) + (vec2(0.125, 0.125) * vec2(index2_8)));
  finalCellPos_7.x = tmpvar_9.x;
  finalCellPos_7.y = (1.0 - tmpvar_9.y);
  a_color.xyz = mix (texture (u_lookup, finalCellPos_4).xyz, texture (u_lookup, finalCellPos_7).xyz, vec3(((64.0 * tmpvar_1.z) - float(tmpvar_2))));
  a_color.w = 1.0;
}

