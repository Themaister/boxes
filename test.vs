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

uniform ModelTransform
{
   mat4 transform;
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
   vec4 world = model.transform * vec4(aVertex, 1.0);
   gl_Position = global_vert.vp * world;

   vout.normal = mat3(model.transform) * aNormal;
   vout.world = world.xyz;
   vout.tex = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

