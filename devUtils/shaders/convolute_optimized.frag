#version 150
out vec4 fragColor;
in vec3 v_localPos;
uniform samplerCube u_environmentMap;
void main ()
{
  float nrSamples_2;
  vec3 right_3;
  vec3 up_4;
  vec3 irradiance_5;
  vec3 normal_6;
  vec3 tmpvar_7;
  tmpvar_7 = normalize(v_localPos);
  normal_6 = tmpvar_7;
  irradiance_5 = vec3(0.0, 0.0, 0.0);
  vec3 tmpvar_8;
  tmpvar_8 = normalize(((vec3(1.0, 0.0, 0.0) * tmpvar_7.zxy) - (vec3(0.0, 0.0, 1.0) * tmpvar_7.yzx)));
  right_3 = tmpvar_8;
  up_4 = normalize(((tmpvar_7.yzx * tmpvar_8.zxy) - (tmpvar_7.zxy * tmpvar_8.yzx)));
  nrSamples_2 = 0.0;
  for (float phi_1 = 0.0; phi_1 < 6.283185; phi_1 += 0.025) {
    for (float theta_9 = 0.0; theta_9 < 1.570796; theta_9 += 0.025) {
      float tmpvar_10;
      tmpvar_10 = cos(theta_9);
      vec3 tmpvar_11;
      tmpvar_11.x = (sin(theta_9) * cos(phi_1));
      tmpvar_11.y = (sin(theta_9) * sin(phi_1));
      tmpvar_11.z = tmpvar_10;
      irradiance_5 = (irradiance_5 + ((texture (u_environmentMap, 
        (((tmpvar_11.x * right_3) + (tmpvar_11.y * up_4)) + (tmpvar_10 * normal_6))
      ).xyz * 
        cos(theta_9)
      ) * sin(theta_9)));
      nrSamples_2 += 1.0;
    };
  };
  irradiance_5 = (irradiance_5 * (3.141593 / nrSamples_2));
  vec4 tmpvar_12;
  tmpvar_12.w = 1.0;
  tmpvar_12.xyz = irradiance_5;
  fragColor = tmpvar_12;
}

