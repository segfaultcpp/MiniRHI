#include "MiniRHI/Texture.hpp"
#include "MiniRHI/Format.hpp"
#ifndef ANDROID
#include <glew/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif

#include <iostream>

namespace minirhi {
	u32 convert_texture_extent(TextureExtent extent) noexcept {
		switch (extent) {
#ifndef ANDROID
		case TextureExtent::e1D: return GL_TEXTURE_1D;
#endif
		case TextureExtent::e2D: return GL_TEXTURE_2D;
		case TextureExtent::e3D: return GL_TEXTURE_3D;
		default: return 0;
		}
	}

	u32 convert_address_mode(TextureAddressMode mode) noexcept {
		switch (mode) {
		case TextureAddressMode::eWrap: return GL_REPEAT;
		case TextureAddressMode::eBorder: return GL_CLAMP_TO_BORDER;
		case TextureAddressMode::eClamp: return GL_CLAMP_TO_EDGE;
		case TextureAddressMode::eMirror: return GL_MIRRORED_REPEAT;
#ifndef ANDROID
		case TextureAddressMode::eMirrorOnce: return GL_MIRROR_CLAMP_TO_EDGE;
#endif
		}

		return 0;
	}

	u32 convert_texture_filter(TextureFilter filter) noexcept {
		switch (filter) {
		case TextureFilter::eLinear: return GL_LINEAR;
		case TextureFilter::eNearest: return GL_NEAREST;
		case TextureFilter::eNearest_MipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
		case TextureFilter::eNearest_MipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureFilter::eLinear_MipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureFilter::eLinear_MipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		default: return 0;
		}
	}

	namespace detail {
		u32 create_texture_impl_(const TextureDesc& desc, const SamplerDesc& sampler) noexcept {
			u32 handle = 0;

			glGenTextures(1, &handle);
		
			auto target = GLenum(convert_texture_extent(desc.extent));
			glBindTexture(target, handle);

			glTexParameteri(target, GL_TEXTURE_WRAP_S, GLint(convert_address_mode(sampler.u)));
			glTexParameteri(target, GL_TEXTURE_WRAP_T, GLint(convert_address_mode(sampler.w)));

			assert(u32(sampler.mag_filter) < u32(TextureFilter::eNearest_MipMapNearest) && "SamplerDesc::mag_filter only accepts TextureFilter::eNeares or TextureFilter::eLinear.");
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GLint(convert_texture_filter(sampler.min_filter)));
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GLint(convert_texture_filter(sampler.mag_filter)));

#ifndef ANDROID
			glTexParameterf(target, GL_TEXTURE_LOD_BIAS, sampler.mip_lod_bias);
#endif
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, sampler.border_color.data());

			if (desc.initial_data != nullptr) {
				auto format = GLint(get_pixel_format(desc.pixel_format));
				auto type = GLint(get_format_type(desc.pixel_format));

				if (desc.extent == TextureExtent::e2D) {
#ifndef _WIN32
					glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
					glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
					glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
					glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

					glTexImage2D(
						target, 
						0, 
						format, 
						GLint(desc.size.width), 
						GLint(desc.size.height), 
						0, 
						format, 
						type, 
						(const void*)desc.initial_data
					);
				}

				if (desc.enable_mips) {
					glGenerateMipmap(target);
				}
			}

			glBindTexture(target, 0);

			return handle;
		}
	}

	void Texture::destroy(Texture& tex) noexcept {
		glDeleteTextures(1, &tex.handle);
	}
}
