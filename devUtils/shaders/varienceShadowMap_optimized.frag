#version 150
uniform sampler2D u_albedoSampler;
uniform int u_hasTexture;
in vec2 v_texCoord;
out vec2 outColor;
void main ()
{
  if ((u_hasTexture != 0)) {
    vec4 tmpvar_1;
    tmpvar_1 = texture (u_albedoSampler, v_texCoord);
    if ((tmpvar_1.w <= 0.1)) {
      discard;
    };
  };
  vec2 tmpvar_2;
  tmpvar_2.x = gl_FragCoord.z;
  tmpvar_2.y = (gl_FragCoord.z * gl_FragCoord.z);
  outColor = tmpvar_2;
}

