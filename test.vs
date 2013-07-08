uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_proj;
};

in vec2 aVertex;
in vec2 aTexCoord;
in vec3 aNormal;

out vec2 vTex;
out vec3 vNormal;
out float vHeight;

void main()
{
   float height = sin(aVertex.x) * cos(aVertex.y);
   gl_Position = vp * vec4(aVertex.x, height, aVertex.y, 1.0);
   vHeight = height;

   vTex = aTexCoord;
   vec4 norm = vp * vec4(aNormal, 0.0);
   vNormal = norm.xyz;
}

