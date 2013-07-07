#ifndef BUFFER_HPP__
#define BUFFER_HPP__

#include "../global.hpp"
#include <stdexcept>
#include <cstring>

class Buffer : public ContextListener, public ContextResource
{
   public:
      enum Flags
      {
         None = 0,
         WriteOnly = 1,
         ReadOnly = 2,
      };

      void init(GLenum target, GLsizei size, GLuint flags, const void *initial_data = nullptr, GLuint index = 0);
      void reset() override;
      void destroyed() override;

      template<typename T>
      bool map(T*& data)
      {
         if (!size || !id || flags == None)
            return false;

         glBindBuffer(target, id);
         void *ptr = glMapBufferRange(target, 0, size, flags == WriteOnly ? (GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT) : GL_MAP_READ_BIT);
         data = reinterpret_cast<T*>(ptr);
         glBindBuffer(target, 0);
         return ptr != nullptr;
      }

      void unmap();
      void bind();
      void unbind();

   private:
      bool alive = false;
      GLenum target = 0;
      GLuint index = 0;
      GLuint flags = 0;
      GLuint id = 0;
      GLsizei size = 0;

      std::vector<uint8_t> temp;

      static GLenum gl_usage_from_flags(GLuint flags);
      void init_buffer(const void *initial_data);
};

#endif

