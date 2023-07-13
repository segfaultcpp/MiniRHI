#include "MiniRHI/Format.hpp"
#ifndef ANDROID
#include <glew/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif

namespace minirhi
{
	u32 get_format_type(Format format) noexcept {
		switch (format) {
		[[fallthrough]]; case Format::eR16_Float:
		case Format::eRG16_Float:
		case Format::eRGB16_Float:
		case Format::eRGBA16_Float:
		case Format::eR32_Float:
		case Format::eRG32_Float:
		case Format::eRGB32_Float:
		case Format::eRGBA32_Float:
			return GL_FLOAT;

		[[fallthrough]]; case Format::eR8_UInt:
		case Format::eRG8_UInt:
		case Format::eRGB8_UInt:
		case Format::eRGBA8_UInt:
			return GL_UNSIGNED_BYTE;

		[[fallthrough]]; case Format::eR16_UInt:
		case Format::eRG16_UInt:
		case Format::eRGB16_UInt:
		case Format::eRGBA16_UInt:
			return GL_UNSIGNED_SHORT;

		[[fallthrough]]; case Format::eR32_UInt:
		case Format::eRG32_UInt:
		case Format::eRGB32_UInt:
		case Format::eRGBA32_UInt:
			return GL_UNSIGNED_INT;

		default:
			return 0;
		}
	}

	u32 get_pixel_format(Format format) noexcept {
		switch (format) {
		[[fallthrough]]; case Format::eR16_Float:
		case Format::eR32_Float:
		case Format::eR8_UInt:
		case Format::eR16_UInt:
		case Format::eR32_UInt:
			return GL_RED;

		[[fallthrough]]; case Format::eRG16_Float:
		case Format::eRG32_Float:
		case Format::eRG8_UInt:
		case Format::eRG16_UInt:
		case Format::eRG32_UInt:
			return GL_RG;

		[[fallthrough]]; case Format::eRGB16_Float:
		case Format::eRGB32_Float:
		case Format::eRGB8_UInt:
		case Format::eRGB16_UInt:
		case Format::eRGB32_UInt:
			return GL_RGB;
			
		[[fallthrough]]; case Format::eRGBA16_Float:
		case Format::eRGBA32_Float:
		case Format::eRGBA8_UInt:
		case Format::eRGBA16_UInt:
		case Format::eRGBA32_UInt:
			return GL_RGBA;
		
		default:
			return 0;
		}
	}

	u32 get_component_count(Format format) noexcept {
		switch (format) {
		[[fallthrough]]; case Format::eR16_Float:
		case Format::eR32_Float:
		case Format::eR8_UInt:
		case Format::eR16_UInt:
		case Format::eR32_UInt:
			return 1;

		[[fallthrough]]; case Format::eRG16_Float:
		case Format::eRG32_Float:
		case Format::eRG8_UInt:
		case Format::eRG16_UInt:
		case Format::eRG32_UInt:
			return 2;

		[[fallthrough]]; case Format::eRGB16_Float:
		case Format::eRGB32_Float:
		case Format::eRGB8_UInt:
		case Format::eRGB16_UInt:
		case Format::eRGB32_UInt:
			return 3;

		[[fallthrough]]; case Format::eRGBA16_Float:
		case Format::eRGBA32_Float:
		case Format::eRGBA8_UInt:
		case Format::eRGBA16_UInt:
		case Format::eRGBA32_UInt:
			return 4;
		
		default:
			return 0;
		}
	}

	size_t get_format_size(Format format) noexcept {
		switch (format) {
		case Format::eR8_UInt:
			return 1;

		[[fallthrough]]; case Format::eR16_Float:
		case Format::eR16_UInt:
			return 2;

		case Format::eRGB8_UInt:
			return 3;

		[[fallthrough]]; case Format::eRGBA8_UInt:
		case Format::eRG16_Float:
		case Format::eR32_Float:
		case Format::eRG16_UInt:
		case Format::eR32_UInt:
			return 4;

		[[fallthrough]]; case Format::eRGB16_Float:
		case Format::eRGB16_UInt:
			return 6;

		[[fallthrough]]; case Format::eRGBA16_Float:
		case Format::eRG32_Float:
		case Format::eRGBA16_UInt:
		case Format::eRG32_UInt:
			return 8;
		
		[[fallthrough]]; case Format::eRGB32_Float:
		case Format::eRGB32_UInt:
			return 12;

		[[fallthrough]]; case Format::eRGBA32_Float:
		case Format::eRGBA32_UInt:
			return 16;

		default:
			return 0;
		}
	}
}
