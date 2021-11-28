#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
uniform vec3 u_lightPos;
uniform float u_farPlane;
in vec4 v_fragPos;
in vec2 v_finalTexCoord;
void main ()
{
  if ((u_hasTexture != 0)) {
    vec4 tmpvar_1;
    tmpvar_1 = texture (u_albedoSampler, v_finalTexCoord);
    if (((tmpvar_1.w * 255.0) < 1.0)) {
      discard;
    };
  };
  vec3 x_2;
  x_2 = (v_fragPos.xyz - u_lightPos);
  gl_FragDepth = (sqrt(dot (x_2, x_2)) / u_farPlane);
}

