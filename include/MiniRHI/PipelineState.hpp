#pragma once
#include <array>
#include <concepts>
#include <vector>
#include <span>
#include <ranges>
#include <cassert>

#include <Core/Core.hpp>

#include "MiniRHI/Format.hpp"
#include "MiniRHI/Shader.hpp"
#include "Shader.hpp"
#include "Format.hpp"

namespace minirhi
{
	enum class PrimitiveTopologyType {
		ePoint,
		eLine,
		eTriangle, // do we need more? 
		eCount,
	};

	template<format::TFormat Fmt>
	struct VtxAttr {
		using Type = Fmt;
	};

	template<typename...>
	struct VtxAttrArr {};

	template<typename T>
	struct GetVtxElemSize;

	template<typename... Attrs>
	struct GetVtxElemSize<VtxAttrArr<Attrs...>> {
		static constexpr std::size_t Value = (Attrs::Type::size() + ...);
	};

	template<typename T>
	inline static constexpr std::size_t kGetVtxElemSize = GetVtxElemSize<T>::Value;

	struct VtxAttrData {
		Format format;
		std::size_t size;
		std::size_t offset;
		std::size_t stride;

		auto operator<=>(const VtxAttrData&) const noexcept = default;
	};

	template<typename... Attrs>
	[[nodiscard]]
	inline consteval auto get_vtx_attr_array() noexcept {
		auto sizes = std::to_array({
			Attrs::Type::size()...
		});
		auto formats = std::to_array({
			Attrs::Type::underlying()...
		});
		std::size_t stride = kGetVtxElemSize<VtxAttrArr<Attrs...>>;

		std::array<VtxAttrData, sizeof...(Attrs)> ret = {};

		std::size_t offset = 0;
		for (std::size_t i : std::views::iota(0u, sizeof...(Attrs))) {
			ret[i] = VtxAttrData {
				.format = formats[i], 
				.size = sizes[i], 
				.offset = offset,
				.stride = stride
			};
			offset += sizes[i];
		}
		return ret;
	}

	template<typename... Attrs>
	[[nodiscard]]
	inline consteval auto get_vtx_attr_array([[maybe_unused]] VtxAttrArr<Attrs...>) noexcept {
		return get_vtx_attr_array<Attrs...>();
	}
	
	namespace tests {
		static_assert(
			get_vtx_attr_array<
				VtxAttr<format::R16Float_t>, 
				VtxAttr<format::R32Float_t>
			>() == 
			std::array { 
				VtxAttrData{ Format::eR16_Float, 2, 0, 6 }, 
				VtxAttrData{ Format::eR32_Float, 4, 2, 6 } 
			}
		);

		static_assert(
			get_vtx_attr_array(
				VtxAttrArr<
					VtxAttr<format::R16Float_t>, 
					VtxAttr<format::R32Float_t>
				>{}
			) == 
			std::array { 
				VtxAttrData{ Format::eR16_Float, 2, 0, 6 }, 
				VtxAttrData{ Format::eR32_Float, 4, 2, 6 } 
			}
		);

		static_assert(
			kGetVtxElemSize<
				VtxAttrArr<
					VtxAttr<format::RGB32Float_t>
				>
			> == format::RGB32Float_t::size()
		);

		static_assert(
			kGetVtxElemSize<
				VtxAttrArr<
					VtxAttr<format::RGB32Float_t>,
					VtxAttr<format::RGBA32Float_t>
				>
			> == format::RGB32Float_t::size() + format::RGBA32Float_t::size()
		);

		static_assert(
			std::same_as<
				VtxAttrArr<
					VtxAttr<format::RGB32Float_t>
				>, 
				VtxAttrArr<
					VtxAttr<format::RGB32Float_t>
				>
			>
		);
	}
	
	struct [[deprecated("Not supported anymore. Use VtxAttr and VtxAttrArr instead.")]] VertexAttributeDesc
	{
		Format Format_;
		u32 Stride;
		u32 Offset;

		VertexAttributeDesc() noexcept = default;

		VertexAttributeDesc(Format format, u32 stride, u32 offset) noexcept
			: Format_(format)
			, Stride(stride)
			, Offset(offset)
		{}

		VertexAttributeDesc& SetFormat(Format format) noexcept
		{
			Format_ = format;
			return *this;
		}

		VertexAttributeDesc& SetStride(u32 stride) noexcept
		{
			Stride = stride;
			return *this;
		}

		VertexAttributeDesc& SetOffset(u32 offset) noexcept
		{
			Offset = offset;
			return *this;
		}
	};

	struct BlendStateDesc {};

	enum class FrontFace {
		eClockWise,
		eCounterClockWise,
	};

	enum class CullFaceMode {
		eFront,
		eBack,
	};

	enum class PolygonMode {
		ePoint,
		eLine,
		eFill,
	};

	struct RasterizerStateDesc {
		FrontFace front;
		bool cull_mode_enabled;
		bool line_smooth_enabled;
		CullFaceMode cull_mode;
		PolygonMode polygon_mode;
		f32 line_width;

		explicit constexpr RasterizerStateDesc() noexcept
			: front(FrontFace::eCounterClockWise)
			, cull_mode_enabled(false)
			, line_smooth_enabled(false)
			, cull_mode(CullFaceMode::eBack)
			, polygon_mode(PolygonMode::eFill)
			, line_width(1.f)
		{}

		explicit constexpr RasterizerStateDesc(
			FrontFace front_face,
			bool enable_cull_mode,
			bool enable_line_smooth,
			CullFaceMode cull_face_mode,
			PolygonMode polygon,
			f32 width
		) noexcept
			: front(front_face)
			, cull_mode_enabled(enable_cull_mode)
			, line_smooth_enabled(enable_line_smooth)
			, cull_mode(cull_face_mode)
			, polygon_mode(polygon)
			, line_width(width)
		{}

		RasterizerStateDesc& set_polygon_mode(PolygonMode mode) noexcept {
			polygon_mode = mode;
			return *this;
		}

		RasterizerStateDesc& enable_cull_mode(bool enable) noexcept {
			cull_mode_enabled = enable;
			return *this;
		}

		RasterizerStateDesc& set_front_face(FrontFace front_face) noexcept {
			front = front_face;
			return *this;
		}

		RasterizerStateDesc& set_cull_face(CullFaceMode mode) noexcept {
			cull_mode = mode;
			return *this;
		}

		RasterizerStateDesc& set_line_width(f32 width) noexcept {
			line_width = width;
			return *this;
		}

		RasterizerStateDesc& enable_line_smooth(bool enable) noexcept {
			line_smooth_enabled = enable;
			return *this;
		}
	};

	struct DepthStencilStateDesc {};

	template<typename Attrs>
	struct PipelineState {
		VtxShaderHandle vs{};
		FragShaderHandle fs{};
		PrimitiveTopologyType topology = PrimitiveTopologyType::eCount;
		RasterizerStateDesc rasterizer{};
		u32 shader_program{};

		explicit constexpr PipelineState() noexcept = default;

		explicit constexpr PipelineState(
			VtxShaderHandle vertex_shader,
			FragShaderHandle fragment_shader,
			PrimitiveTopologyType topology_type,
			const RasterizerStateDesc& rasterizer_state 
		) noexcept
			: vs(vertex_shader)
			, fs(fragment_shader)
			, topology(topology_type)
			, rasterizer(rasterizer_state)
		{}

		PipelineState& set_vertex_shader(VtxShaderHandle vertex_shader) noexcept {
			vs = vertex_shader;
			return *this;
		}

		PipelineState& set_fragment_shader(FragShaderHandle fragment) noexcept {
			fs = fragment;
			return *this;
		}

		PipelineState& set_topology(PrimitiveTopologyType type) noexcept {
			topology = type;
			return *this;
		}

		PipelineState& set_rasterizer(const RasterizerStateDesc& desc) noexcept {
			rasterizer = desc;
			return *this;
		}

		PipelineState& build(bool destroy_shaders = true) noexcept {
			shader_program = ShaderCompiler::link_shaders(vs, fs);
			assert(shader_program != kShaderInvalidHandle);

			if (destroy_shaders) {
				ShaderCompiler::destroy_shaders(vs, fs);
			}

			return *this;
		}
	};
}