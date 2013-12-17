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

               auto model_offset = draw->get_model_transform();
               auto trans = translate(mat4(1.0f), vec3(model_offset.x, model_offset.y, model_offset.z));
               auto scaling = scale(mat4(1.0f), vec3(model_offset.w));
               auto model = trans * scaling;

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

   const RenderQueue::DrawList& RenderQueue::get_draw_list() const
   {
      return draw_list;
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

