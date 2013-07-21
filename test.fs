uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
};

in vec3 vWorldPos;
in vec3 vNormal;

uniform samplerCube skybox; 

out vec4 FragColor;

float saturate(float a)
{
   return clamp(a, 0.0, 1.0);
}

void main()
{
   vec3 vEye = normalize(camera_pos.xyz - vWorldPos);
   vec3 vLight = normalize(light_pos.xyz - vWorldPos);
   vec3 normal = normalize(vNormal);

   float ndotl = saturate(dot(vLight, normal));
   float spec = 0.0;
   if (ndotl > 0.0)
   {
      vec3 half_vec = normalize(vEye + vLight);
      spec = pow(saturate(dot(half_vec, normal)), 50.0);
   }

   vec3 refract_normal = refract(-vEye, normal, 1.4);
   vec3 reflect_normal = reflect(-vEye, normal);

   float incidence = 0.0;
      incidence = dot(refract_normal, -normal);

   vec4 refracted = texture(skybox, refract_normal);
   vec4 reflected = texture(skybox, reflect_normal);

   vec4 col = light_ambient + light_color * spec;
   FragColor = mix(reflected, refracted, incidence) + col * vec4(1.1, 0.8, 0.7, 1.0);
}
