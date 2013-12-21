layout(binding = GLOBAL_FRAGMENT_DATA) uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 camera_vel;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
   vec2 resolution;
} global_frag;

layout(binding = MATERIAL) uniform Material
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
} fin;

#if DIFFUSE_MAP
layout(binding = 0) uniform sampler2D Diffuse;
#endif

out vec4 FragColor;

void main()
{
   vec3 vEye = normalize(global_frag.camera_pos.xyz - fin.world);
   vec3 light_dist = global_frag.light_pos.xyz - fin.world;
   vec3 vLight = normalize(light_dist);
   vec3 normal = normalize(fin.normal);

   float ndotl = max(dot(vLight, normal), 0.0);

   float light_mod = 0.5; // Could attenuate, but don't bother.

   vec3 ambient = material.ambient.rgb;
   vec3 diffuse = light_mod * ndotl * global_frag.light_color.rgb * material.diffuse.rgb;

   FragColor = sqrt(vec4(ambient + diffuse, 1.0));
}

