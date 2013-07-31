uniform GlobalFragmentData
{
   vec4 camera_pos_f;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
};

in VertexData
{
   vec3 vWorldPos;
   vec3 vNormal;
   vec4 vColor;
};

out vec4 FragColor;

void main()
{
   vec3 vEye = normalize(camera_pos_f.xyz - vWorldPos);
   vec3 vLight = normalize(light_pos.xyz - vWorldPos);
   vec3 normal = normalize(vNormal);

   float ndotl = max(dot(vLight, normal), 0.0);

   vec3 mat_color = vec3(0.7, 1.0, 0.7);
   vec3 col = (light_ambient.rgb + ndotl) * mat_color;
   FragColor = vColor * vec4(col, 1.0);
}

