#pragma once
#include "Format.hpp"
#include "RC.hpp"

#include <limits>
#include <span>
#include <array>

#include <glm/vec2.hpp>

namespace minirhi {
	enum class TextureExtent {
		e1D,
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
		static TextureDesc texture_1D(u32 w, Format pixelFormat, const u8* initialData = nullptr, bool enableMips = false) noexcept {
			return TextureDesc{ TextureSize{ w, 1, 1 }, TextureExtent::e2D, pixelFormat, enableMips, initialData };
		}

		[[nodiscard]]
		static TextureDesc texture_2D(u32 w, u32 h, Format pixelFormat, const u8* initialData = nullptr, bool enableMips = false) noexcept {
			return TextureDesc{ TextureSize{ w, h, 1 }, TextureExtent::e2D, pixelFormat, enableMips, initialData };
		}
	};

	enum class TextureAddressMode {
		eWrap,
		eClamp,
		eMirror,
		eBorder,
		eMirrorOnce
	};

	u32 convert_address_mode(TextureAddressMode mode) noexcept;

	enum class TextureFilter {
		eNone,
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
			, min_filter(TextureFilter::eNone)
			, mag_filter(TextureFilter::eNone)
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

	class Texture {
	public:
		u32 handle;
		TextureDesc desc;
		SamplerDesc sampler;

		using DestroyFn = void(Texture& handle);
		static void destroy(Texture& tex) noexcept;

		Texture() noexcept = default;
		Texture(const TextureDesc& desc, const SamplerDesc& sampler) noexcept;
	};

	static_assert(std::is_trivially_copyable_v<Texture> && std::is_trivially_copyable_v<TextureDesc>, "minirhi::Texture and minirhi::TextureDesc must be trivially copyable.");

	using TextureRC = RC<Texture>;
}