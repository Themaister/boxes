#ifndef VERTEX_ARRAY_HPP__
#define VERTEX_ARRAY_HPP__

#include "global.hpp"
#include "buffer.hpp"

namespace GL
{
   class VertexArray : public ContextListener
   {
      public:
         VertexArray() { init(); }
         ~VertexArray() { deinit(); }
         struct Array
         {
            GLuint location;
            GLint size;
            GLenum type;
            GLboolean normalized;
            GLsizei stride;
            GLsizei offset;
            GLuint divisor;
            unsigned buffer_index;
         };

         void reset() override;
         void destroyed() override;

         void setup(const std::vector<Array>& arrays,
               std::vector<Buffer*> array_buffers, Buffer* elem_buffer);

         void bind();
         void unbind();

      private:
         GLuint vao = 0;
         bool alive = false;
         std::vector<Array> arrays;
         std::vector<Buffer*> array_buffers;
         Buffer *elem_buffer = nullptr;

         void setup();
   };
}

#endif

