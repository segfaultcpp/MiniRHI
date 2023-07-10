#include "MiniRHI/Texture.hpp"
#include "MiniRHI/Format.hpp"
#include <glew/glew.h>

#include <format>
#include <iostream>

namespace minirhi {
	u32 convert_texture_extent(TextureExtent extent) noexcept {
		switch (extent) {
		case TextureExtent::e1D: return GL_TEXTURE_1D;
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
		case TextureAddressMode::eMirrorOnce: return GL_MIRROR_CLAMP_TO_EDGE;
		}

		return 0;
	}

	u32 convert_texture_filter(TextureFilter filter) noexcept {
		switch (filter) {
		case TextureFilter::eLinear: return GL_LINEAR;
		case TextureFilter::eNearest: return GL_NEAREST;
		default: return 0;
		}
	}

	Texture::Texture(const TextureDesc& desc, const SamplerDesc& sampler) noexcept {
		glGenTextures(1, &handle);
		
		auto target = GLenum(convert_texture_extent(desc.extent));
		glBindTexture(target, handle);

		glTexParameteri(target, GL_TEXTURE_WRAP_S, GLint(convert_address_mode(sampler.u)));
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GLint(convert_address_mode(sampler.w)));

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GLint(convert_texture_filter(sampler.min_filter)));
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GLint(convert_texture_filter(sampler.mag_filter)));

		glTexParameterf(target, GL_TEXTURE_LOD_BIAS, sampler.mip_lod_bias);
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, sampler.border_color.data());

		if (desc.initial_data != nullptr) {
			auto format = GLint(get_pixel_format(desc.pixel_format));
			auto type = GLint(get_format_type(desc.pixel_format));

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

			if (desc.enable_mips) {
				glGenerateMipmap(target);
			}
		}

		glBindTexture(target, 0);
	}

	void Texture::destroy(Texture& tex) noexcept {
		glDeleteTextures(1, &tex.handle);
	}
}
