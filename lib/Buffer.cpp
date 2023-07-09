#include "MiniRHI/Buffer.hpp"
#include <glew/glew.h>

namespace minirhi
{
    namespace detail {
        GLenum get_buffer_type(BufferType bufferType) {
            switch (bufferType) {
            case BufferType::eVertex:   return GL_ARRAY_BUFFER;
            case BufferType::eIndex:    return GL_ELEMENT_ARRAY_BUFFER;
            case BufferType::eConstant: return GL_UNIFORM_BUFFER;
            case BufferType::eCount:    break;
            }
            return 0;
        }

        u32 create_buffer_(BufferType type, std::size_t size_in_bytes, const void* data) noexcept {
            u32 handle = 0;
            glGenBuffers(1, &handle);

			if (data != nullptr && size_in_bytes != 0)
			{
				GLenum target = get_buffer_type(type);
				glBindBuffer(target, handle);
				glBufferData(target, size_in_bytes, data, GL_STATIC_DRAW);
				glBindBuffer(target, 0);
			}
            return handle;
        }

        void destroy_buffer_(u32 &handle) noexcept {
			glDeleteBuffers(1, &handle);
        }
    }
}