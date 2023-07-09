#pragma once
#include "Format.hpp"
#include "RC.hpp"

#include <span>

namespace minirhi
{
	enum class TextureType
	{
		eTexture1D,
		eTexture2D,
		eTexture3D,
		eUnknown,
		eCount,
	};

	u32 ConvertTextureType(TextureType type) noexcept;

	struct TextureDesc
	{
		u32 Width;
		u32 Height;
		TextureType Type;
		Format PixelFormat;
		bool EnableMips;
		const u8* InitialData;

		TextureDesc() noexcept
			: Width(1u)
			, Height(1u)
			, Type(TextureType::eUnknown)
			, PixelFormat(Format::eUnknown)
			, EnableMips(false)
			, InitialData(nullptr)
		{}

		TextureDesc(u32 w, u32 h, TextureType dimension, Format format, bool enableMips, const u8* initialData) noexcept
			: Width(w)
			, Height(h)
			, Type(dimension)
			, PixelFormat(format)
			, EnableMips(enableMips)
			, InitialData(initialData)
		{}

		static TextureDesc Texture2D(u32 w, u32 h, Format pixelFormat, bool enableMips, const u8* initialData = nullptr) noexcept
		{
			return TextureDesc{ w, h, TextureType::eTexture2D, pixelFormat, enableMips, initialData };
		}
	};

	enum class TextureAddressMode
	{
		eWrap,
		eClamp,
		eMirror,
		eBorder,
		eMirrorOnce
	};

	u32 ConvertAddressMode(TextureAddressMode mode) noexcept;

	enum class TextureFilter
	{
		eNone,
		eNearest,
		eLinear,
	};

	u32 ConvertTextureFilter(TextureFilter filter) noexcept;

	struct SamplerDesc
	{
		f32 BorderColor[4];
		f32 MipLODBias;
		TextureAddressMode U;
		TextureAddressMode V;
		TextureAddressMode W;
		TextureFilter MinFilter;
		TextureFilter MagFilter;

		SamplerDesc() noexcept
			: BorderColor{ 0.f, 0.f, 0.f, 0.f }
			, MipLODBias(0)
			, U(TextureAddressMode::eWrap)
			, V(TextureAddressMode::eWrap)
			, W(TextureAddressMode::eWrap)
			, MinFilter(TextureFilter::eNone)
			, MagFilter(TextureFilter::eNone)
		{}

		SamplerDesc(std::span<f32, 4> borderColor, f32 mipBias, 
			TextureAddressMode u, TextureAddressMode v, TextureAddressMode w, 
			TextureFilter min, TextureFilter mag) noexcept
			: MipLODBias(mipBias)
			, U(u)
			, V(v)
			, W(w)
			, MinFilter(min)
			, MagFilter(mag)
		{
			BorderColor[0] = borderColor[0];
			BorderColor[1] = borderColor[1];
			BorderColor[2] = borderColor[2];
			BorderColor[3] = borderColor[3];
		}

		SamplerDesc& SetBorderColor(f32 r, f32 g, f32 b, f32 a) noexcept
		{
			BorderColor[0] = r;
			BorderColor[1] = g;
			BorderColor[2] = b;
			BorderColor[3] = a;
			return *this;
		}

		SamplerDesc& SetMipLODBias(f32 mipBias) noexcept
		{
			MipLODBias = mipBias;
			return *this;
		}

		SamplerDesc& SetAddressMode(TextureAddressMode u, TextureAddressMode v, TextureAddressMode w) noexcept
		{
			U = u;
			V = v;
			W = w;
			return *this;
		}

		SamplerDesc& SetAddressModeForU(TextureAddressMode u) noexcept
		{
			U = u;
			return *this;
		}

		SamplerDesc& SetAddressModeForV(TextureAddressMode v) noexcept
		{
			V = v;
			return *this;
		}

		SamplerDesc& SetAddressModeForW(TextureAddressMode w) noexcept
		{
			W = w;
			return *this;
		}

		SamplerDesc& SetMinFilter(TextureFilter min) noexcept
		{
			MinFilter = min;
			return *this;
		}

		SamplerDesc& SetMagFilter(TextureFilter mag) noexcept
		{
			MagFilter = mag;
			return *this;
		}
	};

	class Texture
	{
	private:
		u32 _handle;
		TextureDesc _desc;
		SamplerDesc _sampler;

	public:
		using DestroyFn = void(Texture& handle);
		static void Destroy(Texture& tex) noexcept;

	public:
		Texture() noexcept = default;
		Texture(const TextureDesc& desc, const SamplerDesc& sampler) noexcept;

	public:
		operator u32() const noexcept
		{
			return _handle;
		}

		const u32 GetHandle() const noexcept
		{
			return _handle;
		}

	public:
		TextureDesc GetDesc() const noexcept
		{
			return _desc;
		}

		SamplerDesc GetSampler() const noexcept
		{
			return _sampler;
		}

	};

	static_assert(std::is_trivially_copyable_v<Texture> && std::is_trivially_copyable_v<TextureDesc>, "minirhi::Texture and minirhi::TextureDesc must be trivially copyable.");

	using TextureRC = RC<Texture, Texture::Destroy>;
}