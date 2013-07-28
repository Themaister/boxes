#include "vertex_array.hpp"

namespace GL
{
   void VertexArray::setup()
   {
      bind();

      if (array_buffer)
         array_buffer->bind();
      if (elem_buffer)
         elem_buffer->bind();

      for (auto& array : arrays)
      {
         glVertexAttribPointer(array.location, array.size, array.type,
               array.normalized, array.stride, reinterpret_cast<void*>(array.offset));
         glEnableVertexAttribArray(array.location);
         glVertexAttribDivisor(array.location, array.divisor);
      }

      unbind();

      if (array_buffer)
         array_buffer->unbind();
      if (elem_buffer)
         elem_buffer->unbind();
   }

   void VertexArray::bind()
   {
      glBindVertexArray(vao);
   }

   void VertexArray::unbind()
   {
      glBindVertexArray(0);
   }

   void VertexArray::setup(const std::vector<Array>& arrays, Buffer* array_buffer, Buffer* elem_buffer)
   {
      unregister_dependency(this->array_buffer);
      unregister_dependency(this->elem_buffer);
      this->arrays = arrays;
      this->array_buffer = array_buffer;
      this->elem_buffer = elem_buffer;
      register_dependency(this->array_buffer);
      register_dependency(this->elem_buffer);

      if (alive && !arrays.empty())
         setup();
   }

   void VertexArray::reset()
   {
      alive = true;
      glGenVertexArrays(1, &vao);

      if (!arrays.empty())
         setup();
   }

   void VertexArray::destroyed()
   {
      glDeleteVertexArrays(1, &vao);
      vao = 0;
   }
}

