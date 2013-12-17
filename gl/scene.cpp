#include "scene.hpp"

using namespace std;
using namespace glm;

namespace GL
{
   void RenderQueue::set_view_proj(const mat4& vp)
   {
      view_proj = vp;
   }

   void RenderQueue::begin()
   {
      draw_list.clear();
   }

   void RenderQueue::end()
   {
      // Frustum culling.
      draw_list.erase(remove_if(std::begin(draw_list), std::end(draw_list), [this](Renderable* draw) -> bool {
               auto aabb = draw->get_aabb();
               auto model = draw->get_model_transform();

               // Distance in clip space from near plane, plane eq (0, 0, 1, 1).
               auto c = aabb.transform(model).center();
               auto clip = view_proj * vec4(c.x, c.y, c.z, 1.0f);
               draw->set_cache_depth(clip.z + clip.w);

               return !aabb.intersects_clip_space(view_proj * model);
               }),
            std::end(draw_list));

      // Sort based on various criteria.
      sort(std::begin(draw_list), std::end(draw_list),
            [](Renderable* a, Renderable* b) -> bool {
            return a->compare_less(*b);
            });

      //Log::log("%zu draw calls.", draw_list.size());
   }

   void RenderQueue::push(Renderable* elem)
   {
      draw_list.push_back(elem);
   }

   void RenderQueue::render()
   {
      for (auto& elem : draw_list)
         elem->render();
   }
}

