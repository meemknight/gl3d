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
  vec2 finalCellPos_3;
  ivec2 index2_4;
  index2_4.x = (int(mod (tmpvar_2, 8)));
  index2_4.y = (tmpvar_2 / 8);
  vec2 tmpvar_5;
  tmpvar_5 = ((vec2(0.125, 0.125) * tmpvar_1.xy) + (vec2(0.125, 0.125) * vec2(index2_4)));
  finalCellPos_3.x = tmpvar_5.x;
  finalCellPos_3.y = (1.0 - tmpvar_5.y);
  a_color.xyz = texture (u_lookup, finalCellPos_3).xyz;
  a_color.w = 1.0;
}

