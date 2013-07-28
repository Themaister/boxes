uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
};

in vec3 vWorldPos;
in vec3 vNormal;
out vec4 FragColor;

void main()
{
   vec3 vEye = normalize(camera_pos.xyz - vWorldPos);
   vec3 vLight = normalize(light_pos.xyz - vWorldPos);
   vec3 normal = normalize(vNormal);

   float ndotl = max(dot(vLight, normal), 0.0);
   float spec = 0.0;
   if (ndotl > 0.0)
   {
      vec3 half_vec = normalize(vEye + vLight);
      spec = pow(max(dot(half_vec, normal), 0.0), 50.0);
   }

   vec3 mat_color = vec3(1.1, 0.8, 0.7);
   vec3 col = (light_ambient.rgb + ndotl) * mat_color + light_color.rgb * spec * 0.1;
   FragColor = vec4(col, 1.0);
}

