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

uniform VertexSlot1
{
   mat4 model;
};

in vec3 aVertex;
in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vNormal;

void main()
{
   vec4 world = model * vec4(aVertex, 1.0);
   gl_Position = vp * world;
   vWorldPos = world.xyz;
   vNormal = mat3(model) * aNormal;
}

