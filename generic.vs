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
} global_vert;

#define MAX_INSTANCES 64
uniform ModelTransform
{
   mat4 transform[MAX_INSTANCES];
} model;

layout(location = VERTEX) in vec3 aVertex;
layout(location = NORMAL) in vec3 aNormal;
layout(location = TEXCOORD) in vec2 aTexCoord;

out VertexData
{
   vec3 normal;
   vec3 world;
   vec2 tex;
} vout;

void main()
{
#if INSTANCED
   vec4 world = model.transform[gl_InstanceID] * vec4(aVertex, 1.0);
#else
   vec4 world = model.transform[0] * vec4(aVertex, 1.0);
#endif

   gl_Position = global_vert.vp * world;

#if INSTANCED
   vout.normal = mat3(model.transform[gl_InstanceID]) * aNormal;
#else
   vout.normal = mat3(model.transform[0]) * aNormal;
#endif

   vout.world = world.xyz;
   vout.tex = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

