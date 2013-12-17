#include "mesh.hpp"

using namespace std;
using namespace glm;

namespace GL
{
   vec3 AABB::center() const { return base + vec3(0.5f) * offset; }

   // Assumes we're not using projective geometry ... It gets troublesome to handle flipped-sign W.
   // Transform all corners of the AABB then create a new AABB based on the transformed result.
   AABB AABB::transform(const mat4& mat) const
   {
      AABB aabb;
      vec3 corners[8];
      for (unsigned i = 0; i < 8; i++)
      {
         vec3 c = corner(i);
         vec4 c_trans = mat * vec4(c.x, c.y, c.z, 1.0f);
         corners[i] = vec3(c_trans.x, c_trans.y, c_trans.z) / vec3(c_trans.w);
      }

      vec3 minimum = corners[0];
      vec3 maximum = minimum;
      for (auto& v : corners)
      {
         minimum = min(minimum, v);
         maximum = max(maximum, v);
      }

      aabb.base = minimum;
      aabb.offset = maximum - minimum;
      return aabb;
   }

   bool AABB::intersects_clip_space(const mat4& view_proj) const
   {
      vec4 corners[8];
      for (unsigned i = 0; i < 8; i++)
      {
         vec3 c = corner(i);
         corners[i] = view_proj * vec4(c.x, c.y, c.z, 1.0f);
      }

      // Plane equations for clip space.
      const vec4 eqs[] = {
         vec4{ 1,  1,  1,  1}, // Left
         vec4{-1,  0,  0,  1}, // Right
         vec4{ 0, -1,  0,  1}, // Top
         vec4{ 0,  1,  0,  1}, // Bottom
         vec4{ 0,  0,  1,  1}, // Near
         vec4{ 0,  0, -1,  1}, // Far
      };

      for (auto& eq : eqs)
      {
         bool culled = true;
         for (auto& c : corners)
         {
            if (dot(c, eq) > 0.0f)
            {
               culled = false;
               break;
            }
         }

         if (culled)
            return false;
      }

      return true;
   }

   vec3 AABB::corner(unsigned corner) const
   {
      vec3 b = base;
      if (corner & 4)
         b.z += offset.z;
      if (corner & 2)
         b.y += offset.y;
      if (corner & 1)
         b.x += offset.x;

      return b;
   }
}

