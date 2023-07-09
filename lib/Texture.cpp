#include "MiniRHI/Texture.hpp"
#include "MiniRHI/Format.hpp"
#include <glew/glew.h>

#include <format>
#include <iostream>

namespace minirhi
{
	u32 ConvertTextureType(TextureType type) noexcept
	{
		switch (type)
		{
		case TextureType::eTexture1D: return GL_TEXTURE_1D;
		case TextureType::eTexture2D: return GL_TEXTURE_2D;
		case TextureType::eTexture3D: return GL_TEXTURE_3D;
		}
		return 0;
	}

	u32 ConvertAddressMode(TextureAddressMode mode) noexcept
	{
		switch (mode)
		{
		case TextureAddressMode::eWrap: return GL_REPEAT;
		case TextureAddressMode::eBorder: return GL_CLAMP_TO_BORDER;
		case TextureAddressMode::eClamp: return GL_CLAMP_TO_EDGE;
		case TextureAddressMode::eMirror: return GL_MIRRORED_REPEAT;
		case TextureAddressMode::eMirrorOnce: return GL_MIRROR_CLAMP_TO_EDGE;
		}

		return 0;
	}

	u32 ConvertTextureFilter(TextureFilter filter) noexcept
	{
		switch (filter)
		{
		case TextureFilter::eLinear: return GL_LINEAR;
		case TextureFilter::eNearest: return GL_NEAREST;
		}

		return 0;
	}



	Texture::Texture(const TextureDesc& desc, const SamplerDesc& sampler) noexcept
	{
		glGenTextures(1, &_handle);
		
		u32 target = ConvertTextureType(desc.Type);
		glBindTexture(target, _handle);

		glTexParameteri(target, GL_TEXTURE_WRAP_S, ConvertAddressMode(sampler.U));
		glTexParameteri(target, GL_TEXTURE_WRAP_T, ConvertAddressMode(sampler.W));

		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, ConvertTextureFilter(sampler.MinFilter));
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, ConvertTextureFilter(sampler.MagFilter));

		glTexParameterf(target, GL_TEXTURE_LOD_BIAS, sampler.MipLODBias);
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, sampler.BorderColor);

		if (desc.InitialData != nullptr)
		{
			GLenum pixel = get_pixel_format(desc.PixelFormat);
			GLenum format = get_pixel_format(desc.PixelFormat);

			glTexImage2D(target, 0, 
				pixel, desc.Width, desc.Height, 0, 
				pixel, format, 
				(const void*)desc.InitialData);

			if (desc.EnableMips)
			{
				glGenerateMipmap(target);
			}
		}

		glBindTexture(target, 0);
	}

	void Texture::Destroy(Texture& tex) noexcept
	{
		glDeleteTextures(1, &tex._handle);
	}
}
