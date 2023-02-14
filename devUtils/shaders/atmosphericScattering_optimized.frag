#version 150
uniform vec3 u_lightPos;
uniform vec3 u_color1;
uniform vec3 u_color2;
uniform vec3 u_groundColor;
uniform float u_g;
uniform int u_useGround;
in vec3 v_localPos;
out vec3 fragColor;
void main ()
{
  vec3 tmpvar_1;
  tmpvar_1 = normalize(v_localPos);
  vec3 tmpvar_2;
  tmpvar_2 = normalize(u_lightPos);
  float tmpvar_3;
  tmpvar_3 = max (tmpvar_1.y, 0.0);
  float tmpvar_4;
  tmpvar_4 = (1.0 - tmpvar_3);
  float tmpvar_5;
  tmpvar_5 = (u_g * u_g);
  float tmpvar_6;
  tmpvar_6 = dot (tmpvar_2, tmpvar_1);
  vec3 tmpvar_7;
  tmpvar_7 = (((u_color1 + 
    ((((1.5 * 
      ((1.0 - tmpvar_5) / (2.0 + tmpvar_5))
    ) * (1.0 + 
      (tmpvar_6 * tmpvar_6)
    )) / pow ((
      (1.0 + tmpvar_5)
     - 
      ((2.0 * u_g) * tmpvar_6)
    ), 1.5)) * u_color2)
  ) + (
    ((1.0 - abs(tmpvar_2.y)) * u_color2)
   * 
    pow (tmpvar_4, 16.0)
  )) + (pow (tmpvar_4, 16.0) * u_color2));
  if (((tmpvar_3 < 0.02) && (u_useGround != 0))) {
    float tmpvar_8;
    tmpvar_8 = min (max ((tmpvar_3 / 0.02), 0.0), 1.0);
    fragColor = mix (u_groundColor, tmpvar_7, vec3((tmpvar_8 * (tmpvar_8 * tmpvar_8))));
  } else {
    fragColor = tmpvar_7;
  };
}

