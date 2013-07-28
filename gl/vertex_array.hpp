#ifndef VERTEX_ARRAY_HPP__
#define VERTEX_ARRAY_HPP__

#include "global.hpp"
#include "buffer.hpp"

namespace GL
{
   class VertexArray : public ContextListener
   {
      public:
         struct Array
         {
            GLuint location;
            GLint size;
            GLenum type;
            GLboolean normalized;
            GLsizei stride;
            GLsizei offset;
            GLuint divisor;
         };

         void reset() override;
         void destroyed() override;

         void setup(const std::vector<Array>& arrays, Buffer* array_buffer, Buffer* elem_buffer);

         void bind();
         void unbind();

      private:
         GLuint vao = 0;
         bool alive = false;
         std::vector<Array> arrays;
         Buffer *array_buffer = nullptr;
         Buffer *elem_buffer = nullptr;

         void setup();
   };
}

#endif

