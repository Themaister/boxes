in vec2 vCoord;
uniform sampler2D heightmap;

out vec4 FragColor;

void main()
{
   ivec2 coord = ivec2(vCoord);
   ivec4 offset = ivec4(0, 1, 0, -1);
   float h0 = texelFetch(heightmap, coord + offset.wx, 0).r;
   float h1 = texelFetch(heightmap, coord + offset.yx, 0).r;
   float h2 = texelFetch(heightmap, coord + offset.xw, 0).r;
   float h3 = texelFetch(heightmap, coord + offset.xy, 0).r;

   float dx = h1 - h0;
   float dz = h3 - h2;
   FragColor = vec4(dx, dz, 0.0, 0.0);
}

