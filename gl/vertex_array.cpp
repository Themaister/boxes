#include "vertex_array.hpp"

using namespace std;

namespace GL
{
   void VertexArray::setup()
   {
      bind();

      if (elem_buffer)
         elem_buffer->bind();

      for (auto& array : arrays)
      {
         array_buffers[array.buffer_index]->bind();
         glVertexAttribPointer(array.location, array.size, array.type,
               array.normalized, array.stride, reinterpret_cast<void*>(array.offset));
         glEnableVertexAttribArray(array.location);
         glVertexAttribDivisor(array.location, array.divisor);
         array_buffers[array.buffer_index]->unbind();
      }

      unbind();

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

   void VertexArray::setup(const vector<Array>& arrays, vector<Buffer*> array_buffers, Buffer* elem_buffer)
   {
      for (auto& buffer : this->array_buffers)
         unregister_dependency(buffer);
      unregister_dependency(this->elem_buffer);

      this->arrays = arrays;
      this->array_buffers = move(array_buffers);
      this->elem_buffer = elem_buffer;

      for (auto& buffer : this->array_buffers)
         register_dependency(buffer);
      register_dependency(this->elem_buffer);

      if (alive)
      {
         destroyed();
         reset();
         if (!arrays.empty())
            setup();
      }
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

