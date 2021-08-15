#version 150
out vec4 FragColor;
in vec3 v_localPos;
uniform sampler2D u_equirectangularMap;
void main ()
{
  vec3 tmpvar_1;
  tmpvar_1 = normalize(v_localPos);
  vec2 uv_2;
  float tmpvar_3;
  float tmpvar_4;
  tmpvar_4 = (min (abs(
    (tmpvar_1.z / tmpvar_1.x)
  ), 1.0) / max (abs(
    (tmpvar_1.z / tmpvar_1.x)
  ), 1.0));
  float tmpvar_5;
  tmpvar_5 = (tmpvar_4 * tmpvar_4);
  tmpvar_5 = (((
    ((((
      ((((-0.01213232 * tmpvar_5) + 0.05368138) * tmpvar_5) - 0.1173503)
     * tmpvar_5) + 0.1938925) * tmpvar_5) - 0.3326756)
   * tmpvar_5) + 0.9999793) * tmpvar_4);
  tmpvar_5 = (tmpvar_5 + (float(
    (abs((tmpvar_1.z / tmpvar_1.x)) > 1.0)
  ) * (
    (tmpvar_5 * -2.0)
   + 1.570796)));
  tmpvar_3 = (tmpvar_5 * sign((tmpvar_1.z / tmpvar_1.x)));
  if ((abs(tmpvar_1.x) > (1e-8 * abs(tmpvar_1.z)))) {
    if ((tmpvar_1.x < 0.0)) {
      if ((tmpvar_1.z >= 0.0)) {
        tmpvar_3 += 3.141593;
      } else {
        tmpvar_3 = (tmpvar_3 - 3.141593);
      };
    };
  } else {
    tmpvar_3 = (sign(tmpvar_1.z) * 1.570796);
  };
  vec2 tmpvar_6;
  tmpvar_6.x = tmpvar_3;
  tmpvar_6.y = (sign(tmpvar_1.y) * (1.570796 - (
    sqrt((1.0 - abs(tmpvar_1.y)))
   * 
    (1.570796 + (abs(tmpvar_1.y) * (-0.2146018 + (
      abs(tmpvar_1.y)
     * 
      (0.08656672 + (abs(tmpvar_1.y) * -0.03102955))
    ))))
  )));
  uv_2 = (tmpvar_6 * vec2(0.1591, 0.3183));
  uv_2 = (uv_2 + 0.5);
  vec4 tmpvar_7;
  tmpvar_7.w = 1.0;
  tmpvar_7.xyz = texture (u_equirectangularMap, uv_2).xyz;
  FragColor = tmpvar_7;
}

