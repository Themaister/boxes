in vec2 aVertex;
out vec2 vCoord;

uniform sampler2D heightmap;

void main()
{
   gl_Position = vec4(aVertex.x, aVertex.y, 1.0, 1.0);
   vCoord = (aVertex + 1.0) * 0.5 * vec2(textureSize(heightmap, 0));
}
