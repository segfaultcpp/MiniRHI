#pragma once

#include "MiniRHI/Format.hpp"
#include <Core/Core.hpp>

namespace minirhi
{
	enum class Format
	{
		eR16_Float, // 2
		eRG16_Float, // 4
		eRGB16_Float, // 6
		eRGBA16_Float, // 8

		eR32_Float, // 4
		eRG32_Float, // 8
		eRGB32_Float, // 12
		eRGBA32_Float, // 16

		eR8_UInt, // 1
		eRG8_UInt, // 2
		eRGB8_UInt, // 3
		eRGBA8_UInt, // 4

		eR16_UInt, // 2
		eRG16_UInt, // 4
		eRGB16_UInt, // 6
		eRGBA16_UInt, // 8

		eR32_UInt, // 4
		eRG32_UInt, // 8
		eRGB32_UInt, // 12
		eRGBA32_UInt, // 16



		eUnknown,
		eCount,
	};

	u32 get_component_count(Format format) noexcept;
	u32 get_format_type(Format format) noexcept;
	u32 get_pixel_format(Format format) noexcept;
	size_t get_format_size(Format format) noexcept;

	namespace format {
		struct FormatBase {};

		template<typename T>
		concept TFormat = std::derived_from<T, FormatBase> && 
		requires(::minirhi::Format f, ::std::size_t s) {
			f = T::underlying();
			s = T::size();
		};
	
		#define MINIRHI_DECLARE_FORMAT_TYPE_(Name, Underlying, Size) \
		struct Name : ::minirhi::format::FormatBase { \
			[[nodiscard]] \
			static constexpr ::minirhi::Format underlying() noexcept { \
				return ::minirhi::Format::Underlying; \
			} \
			static constexpr ::std::size_t size() noexcept {\
				return Size; \
			}\
		};

		MINIRHI_DECLARE_FORMAT_TYPE_(R16Float_t, eR16_Float, 2);
		MINIRHI_DECLARE_FORMAT_TYPE_(RG16Float_t, eRG16_Float, 4);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGB16Float_t, eRGB16_Float, 6);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGBA16Float_t, eRGBA16_Float, 8);

		MINIRHI_DECLARE_FORMAT_TYPE_(R32Float_t, eR32_Float, 4);
		MINIRHI_DECLARE_FORMAT_TYPE_(RG32Float_t, eRG32_Float, 8);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGB32Float_t, eRGB32_Float, 12);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGBA32Float_t, eRGBA32_Float, 16);

		MINIRHI_DECLARE_FORMAT_TYPE_(R8UInt_t, eR8_UInt, 1);
		MINIRHI_DECLARE_FORMAT_TYPE_(RG8UInt_t, eRG8_UInt, 2);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGB8UInt_t, eRGB8_UInt, 3);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGBA8UInt_t, eRGBA8_UInt, 4);

		MINIRHI_DECLARE_FORMAT_TYPE_(R16UInt_t, eR16_UInt, 2);
		MINIRHI_DECLARE_FORMAT_TYPE_(RG16UInt_t, eRG16_UInt, 4);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGB16UInt_t, eRGB16_UInt, 6);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGBA16UInt_t, eRGBA16_UInt, 8);

		MINIRHI_DECLARE_FORMAT_TYPE_(R32UInt_t, eR32_UInt, 4);
		MINIRHI_DECLARE_FORMAT_TYPE_(RG32UInt_t, eRG32_UInt, 8);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGB32UInt_t, eRGB32_UInt, 12);
		MINIRHI_DECLARE_FORMAT_TYPE_(RGBA32UInt_t, eRGBA32_UInt, 16);
	}

	static_assert(format::R16Float_t::underlying() == Format::eR16_Float);
}