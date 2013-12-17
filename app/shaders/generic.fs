uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
} global_frag;

uniform Material
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float specular_power;
} material;

in VertexData
{
   vec3 normal;
   vec3 world;
   vec2 tex;
} fin;

#if DIFFUSE_MAP
uniform sampler2D Diffuse;
#endif

out vec4 FragColor;

void main()
{
   vec3 vEye = normalize(global_frag.camera_pos.xyz - fin.world);
   vec3 light_dist = global_frag.light_pos.xyz - fin.world;
   vec3 vLight = normalize(light_dist);
   vec3 normal = normalize(fin.normal);

   float ndotl = max(dot(vLight, normal), 0.0);

   float light_mod = 1000.0 / (1.0 + dot(light_dist, light_dist));

   vec3 specular = vec3(0.0);
   if (ndotl > 0.0)
   {
      vec3 half_vec = normalize(vLight + vEye);
      specular = light_mod * material.specular.xyz * pow(max(dot(half_vec, normal), 0.0), material.specular_power);
   }

#if DIFFUSE_MAP
   vec4 diffuse_term = texture(Diffuse, fin.tex);
   diffuse_term.rgb *= diffuse_term.rgb;
#else
   vec4 diffuse_term = vec4(0.0);
#endif

   vec3 ambient = global_frag.light_ambient.rgb * mix(material.ambient.rgb, diffuse_term.rgb, diffuse_term.a);

   vec3 diffuse = light_mod * ndotl * global_frag.light_color.rgb * mix(material.diffuse.rgb, diffuse_term.rgb, diffuse_term.a);

   FragColor = sqrt(vec4(ambient + diffuse + specular, 1.0));
}

