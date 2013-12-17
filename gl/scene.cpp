#include "scene.hpp"
#include <algorithm>

using namespace std;
using namespace glm;

namespace GL
{
   void render_draw_list(const mat4& view_proj, const DrawList& list,
         std::function<void (Shader*, std::uintptr_t shader_key)> start_shader)
   {
      DrawList culled_list;
      Log::log("Drawlist size: %zu.", list.size());

      // Frustum cull first.
      for (auto& drawable : list)
      {
         auto transformed_aabb = drawable.aabb.transform(drawable.model_transform);

#if 1
         // Check if transformed AABB is within clip space.
         if (transformed_aabb.intersects_clip_space(view_proj))
         {
            auto draw = drawable;
            auto c = transformed_aabb.center();
            auto clip = view_proj * vec4(c.x, c.y, c.z, 1.0f);
            draw.depth = clip.z / clip.w; // Non-linear depth. Will be a bit wonky when camera space depth is positive, but shouldn't be a problem.
            culled_list.push_back(draw);
         }
#else
         culled_list.push_back(drawable);
#endif
      }

      Log::log("Culled drawlist size: %zu.", culled_list.size());

      // Sort on shader, shader_key, then depth (aabb).
      sort(begin(culled_list), end(culled_list),
            [](const Drawable& a, const Drawable& b) -> bool {
               if (a.shader < b.shader)
                  return true;
               else if (a.shader_key < b.shader_key)
                  return true;
               else if (a.depth < b.depth)
                  return true;
               else
                  return false;
            });

      Shader *shader = nullptr;
      uintptr_t shader_key = 0;
      for (auto& draw : culled_list)
      {
         if (shader != draw.shader || shader_key != draw.shader_key)
         {
            shader = draw.shader;
            shader_key = draw.shader_key;
            start_shader(shader, shader_key);
         }

         draw.draw();
      }
   }
}

