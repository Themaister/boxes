#include "buffer.hpp"

void Buffer::init(GLenum target, GLsizei size, GLuint flags, const void *initial_data, GLuint index)
{
   this->target = target;
   this->size = size;
   this->flags = flags;
   this->index = index;

   if (alive)
      init_buffer(initial_data);

   if (initial_data)
   {
      // Keep the data for later.
      temp.resize(size);
      std::memcpy(temp.data(), initial_data, size);
   }
   else
      temp.clear();
}

void Buffer::reset()
{
   alive = true;
   glGenBuffers(1, &id);

   if (size && target)
      init_buffer(temp.empty() ? nullptr : temp.data());
}

void Buffer::destroyed()
{
   alive = false;
   if (id)
      glDeleteBuffers(1, &id);
   id = 0;
}

void Buffer::unmap()
{
   glBindBuffer(target, id);
   glUnmapBuffer(target);
   glBindBuffer(target, 0);
}

void Buffer::bind()
{
   if (target == GL_UNIFORM_BUFFER)
      glBindBufferBase(GL_UNIFORM_BUFFER, index, id);
   else
      glBindBuffer(target, id);
}

void Buffer::unbind()
{
   if (target == GL_UNIFORM_BUFFER)
      glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
   else
      glBindBuffer(target, 0);
}

GLenum Buffer::gl_usage_from_flags(GLuint flags)
{
   switch (flags)
   {
      case WriteOnly: return GL_DYNAMIC_DRAW;
      case ReadOnly: return GL_DYNAMIC_READ;
      default: return GL_STATIC_DRAW;
   }
}

void Buffer::init_buffer(const void *initial_data)
{
   glBindBuffer(target, id);
   glBufferData(target, size, initial_data, gl_usage_from_flags(flags));
   glBindBuffer(target, 0);
}

