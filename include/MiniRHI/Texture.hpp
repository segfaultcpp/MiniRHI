#pragma once
#include "Format.hpp"
#include "RC.hpp"

#include <limits>
#include <span>
#include <array>

#include <glm/vec2.hpp>

namespace minirhi {
	enum class TextureExtent {
#ifndef ANDROID
		e1D,
#endif
		e2D,
		e3D,
		eUnknown,
		eCount,
	};

	u32 convert_texture_extent(TextureExtent extent) noexcept;

	struct TextureSize {
		u32 width;
		u32 height;
		u32 array_size;
	};

	struct TextureDesc {
		TextureSize size;
		TextureExtent extent;
		Format pixel_format;
		bool enable_mips;
		const u8* initial_data;

		explicit TextureDesc() noexcept
			: size({0, 0, 0})
			, extent(TextureExtent::eUnknown)
			, pixel_format(Format::eUnknown)
			, enable_mips(false)
			, initial_data(nullptr)
		{}

		explicit TextureDesc(const TextureSize& texture_size, TextureExtent texture_extent, Format format, bool enableMips, const u8* initialData) noexcept
			: size(texture_size)
			, extent(texture_extent)
			, pixel_format(format)
			, enable_mips(enableMips)
			, initial_data(initialData)
		{}

		[[nodiscard]]
		static TextureDesc texture_1D(u32 w, Format format, const u8* data = nullptr, bool enable_mips = false) noexcept {
			return TextureDesc{ TextureSize{ w, 1, 1 }, TextureExtent::e2D, format, enable_mips, data };
		}

		[[nodiscard]]
		static TextureDesc texture_2D(u32 w, u32 h, Format format, const u8* data = nullptr, bool enable_mips = false) noexcept {
			return TextureDesc{ TextureSize{ w, h, 1 }, TextureExtent::e2D, format, enable_mips, data };
		}
	};

	enum class TextureAddressMode {
		eWrap,
		eClamp,
		eMirror,
		eBorder,
#ifndef ANDROID
		eMirrorOnce
#endif
	};

	u32 convert_address_mode(TextureAddressMode mode) noexcept;

	enum class TextureFilter {
		eNearest,
		eLinear,
	};

	u32 convert_texture_filter(TextureFilter filter) noexcept;

	struct SamplerDesc {
		std::array<f32, 4> border_color;
		f32 mip_lod_bias;
		TextureAddressMode u;
		TextureAddressMode v;
		TextureAddressMode w;
		TextureFilter min_filter;
		TextureFilter mag_filter;

		SamplerDesc() noexcept
			: border_color{ 0.f, 0.f, 0.f, 0.f }
			, mip_lod_bias(0)
			, u(TextureAddressMode::eWrap)
			, v(TextureAddressMode::eWrap)
			, w(TextureAddressMode::eWrap)
			, min_filter(TextureFilter::eLinear)
			, mag_filter(TextureFilter::eLinear)
		{}

		SamplerDesc(
			std::span<f32, 4> bord_color, 
			f32 mip_bias, 
			TextureAddressMode u_axis, 
			TextureAddressMode v_axis, 
			TextureAddressMode w_axis, 
			TextureFilter min, 
			TextureFilter mag
		) noexcept
			: border_color()
			, mip_lod_bias(mip_bias)
			, u(u_axis)
			, v(v_axis)
			, w(w_axis)
			, min_filter(min)
			, mag_filter(mag)
		{
			std::copy(bord_color.begin(), bord_color.end(), border_color.begin());
		}
	};

	inline static constexpr u32 kInvalidTextureHandle = std::numeric_limits<u32>::max();

	namespace detail {
		u32 create_texture_impl_(const TextureDesc& desc, const SamplerDesc& sampler) noexcept;
	}

	struct Texture {
		u32 handle{};
		TextureDesc desc;
		SamplerDesc sampler;

		static void destroy(Texture& tex) noexcept;

		explicit Texture() noexcept = default;

		explicit Texture(const TextureDesc& tex_desc, const SamplerDesc& tex_sampler) noexcept
			: handle(detail::create_texture_impl_(tex_desc, tex_sampler))
			, desc(tex_desc)
			, sampler(tex_sampler)
		{}
	};

	using TextureRC = RC<Texture>;

	inline TextureRC make_texture_rc(const TextureDesc& desc, const SamplerDesc& sampler) noexcept {
		return TextureRC{ desc, sampler };
	}

	inline TextureRC make_texture_2d_rc(const SamplerDesc& sampler, u32 w, u32 h, Format format, const u8* data, bool enable_mips = false) noexcept {
		return TextureRC{ TextureDesc::texture_2D(w, h, format, data, enable_mips), sampler };
	}
}