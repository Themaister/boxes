#ifndef BUFFER_HPP__
#define BUFFER_HPP__

#include "../global.hpp"
#include <stdexcept>

class Buffer : public ContextListener
{
   public:
      enum Flags
      {
         None = 0,
         WriteOnly = 1,
         ReadOnly = 2,
      };

      void init(GLenum target, GLsizei size, GLuint flags, const void *initial_data = nullptr, GLuint index = 0)
      {
         this->target = target;
         this->size = size;
         this->flags = flags;
         this->index = index;

         if (alive)
            init_buffer(initial_data);
         else if (!alive && initial_data)
            throw std::logic_error("Cannot create immutable buffer as buffer isn't initialized yet.\n");
      }

      void reset() override
      {
         alive = true;
         glGenBuffers(1, &id);

         if (size && target)
            init_buffer(nullptr);
      }

      void destroyed() override
      {
         alive = false;
         if (id)
            glDeleteBuffers(1, &id);
         id = 0;
      }

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

      void unmap()
      {
         glBindBuffer(target, id);
         glUnmapBuffer(target);
         glBindBuffer(target, 0);
      }

      void bind()
      {
         if (target == GL_UNIFORM_BUFFER)
            glBindBufferBase(GL_UNIFORM_BUFFER, index, id);
         else
            glBindBuffer(target, id);
      }

      void unbind()
      {
         if (target == GL_UNIFORM_BUFFER)
            glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
         else
            glBindBuffer(target, 0);
      }

   private:
      bool alive = false;
      GLenum target = 0;
      GLuint index = 0;
      GLuint flags = 0;
      GLuint id = 0;
      GLsizei size = 0;

      static GLenum gl_usage_from_flags(GLuint flags)
      {
         switch (flags)
         {
            case WriteOnly: return GL_DYNAMIC_DRAW;
            case ReadOnly: return GL_DYNAMIC_READ;
            default: return GL_STATIC_DRAW;
         }
      }

      void init_buffer(const void *initial_data)
      {
         glBindBuffer(target, id);
         glBufferData(target, size, initial_data, gl_usage_from_flags(flags));
         glBindBuffer(target, 0);
      }
};

#endif

