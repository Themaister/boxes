uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 view_nt;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_view_nt;
   mat4 inv_proj;
   vec4 camera_pos;
};

uniform ModelTransform
{
   mat4 model;
};

in vec2 aVertex;

out VertexData
{
   vec3 vWorldPos;
   vec3 vNormal;
   vec4 vColor;
};

uniform sampler2D normalmap;

void main()
{
   float y = 15.0;

   vec4 val = texelFetch(normalmap, ivec2(aVertex), 0);
   float height = val.z * y;

   vec2 normal = (val.xy - 0.5) * 2.0 * y;
   vNormal = normalize(vec3(-normal.x, 2.0, -normal.y));

   vec4 world = model * vec4(aVertex.x, height, aVertex.y, 1.0);
   gl_Position = vp * world;
   vWorldPos = world.xyz;
   vColor = vec4(0.25 * height + 0.25);
}

