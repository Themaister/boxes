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

#if INSTANCED
#define MAX_INSTANCES 1024
uniform ModelTransform
{
   vec4 offset[MAX_INSTANCES]; // vec4(vec3(offset), scale)
} model;
#else
uniform ModelTransform
{
   vec4 transform;
} model;
#endif

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
   vec4 world = vec4(model.offset[gl_InstanceID].xyz + model.offset[gl_InstanceID].w * aVertex, 1.0);
#else
   vec4 world = model.offset + vec4(model.offset.w * aVertex, 1.0);
#endif

   gl_Position = global_vert.vp * world;

#if INSTANCED
   vout.normal = aNormal;
#else
   vout.normal = aNormal;
#endif

   vout.world = world.xyz;
   vout.tex = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

