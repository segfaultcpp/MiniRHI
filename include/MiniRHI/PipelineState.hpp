#pragma once
#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <span>
#include <cassert>

#include <Core/Core.hpp>

#include "MiniRHI/Format.hpp"
#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/Shader.hpp"
#include "MiniRHI/Texture.hpp"
#include "Shader.hpp"
#include "Format.hpp"
#include "glm/ext/matrix_float4x4.hpp"

namespace minirhi
{
	enum class PrimitiveTopologyType {
		ePoint = 0,
		eLine,
		eTriangle, 
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
		for (std::size_t i = 0; i < sizeof...(Attrs); i++) {
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

	template<typename Type, typename Name>
	struct Slot;

	template<typename Type>
	struct Slot<Type, CTString<FixedString("")>>;

	template<typename Name>
	struct Slot<CTString<FixedString(glsl::TypeNames::kSampler2D)>, Name> {
		TextureRC value{};

		explicit constexpr Slot() noexcept = default;
		explicit constexpr Slot(TextureRC texture_handle) noexcept 
			: value(std::move(texture_handle))
		{}
	};
	template<FixedString Name>
	using Texture2DSlot = Slot<CTString<FixedString(glsl::TypeNames::kSampler2D)>, CTString<Name>>;

	template<typename Name>
	struct Slot<CTString<FixedString(glsl::TypeNames::kUInt)>, Name> {
		u32 value = 0;

		explicit constexpr Slot() noexcept = default;
		explicit constexpr Slot(u32 v) noexcept 
			: value(v)
		{}
	};
	template<FixedString Name>
	using UIntSlot = Slot<CTString<FixedString(glsl::TypeNames::kUInt)>, CTString<Name>>;

	template<typename Name>
	struct Slot<CTString<FixedString(glsl::TypeNames::kFloat)>, Name> {
		f32 value = 0.f;

		explicit constexpr Slot() noexcept = default;
		explicit constexpr Slot(f32 v) noexcept 
			: value(v)
		{}
	};
	template<FixedString Name>
	using FloatSlot = Slot<CTString<FixedString(glsl::TypeNames::kFloat)>, CTString<Name>>;

	template<typename Name>
	struct Slot<CTString<FixedString(glsl::TypeNames::kMat4)>, Name> {
		glm::mat4 value;

		explicit constexpr Slot() noexcept = default;
		explicit constexpr Slot(const glm::mat4& v) noexcept
			: value(v)
		{}
	};
	template<FixedString Name>
	using Mat4Slot = Slot<CTString<FixedString(glsl::TypeNames::kMat4)>, CTString<Name>>;

	template<typename... Slots>
	struct BindingSet {
		using Tuple = std::tuple<Slots...>;
		static constexpr std::size_t kSlotCount = sizeof...(Slots);
		static constexpr bool kIsEmpty = kSlotCount == 0;
		Tuple slots{};

		template<typename T>
		auto& get_slot() noexcept {
			return std::get<T>(slots);
		}

		template<typename T>
		[[nodiscard]]
		const auto& get_slot() const noexcept {
			return std::get<T>(slots);
		}

#define MINIRHI_DECLARE_BINDING_SLOT_GETTER_INLINED_(Type, Name) \
template<FixedString Uniform>\
Type<Uniform>& get_##Name##_slot() noexcept { \
	return get_slot<Type<Uniform>>(); \
} \
template<FixedString Uniform>\
const Type<Uniform>& get_##Name##_slot() const noexcept { \
	return get_slot<Type<Uniform>>(); \
}

		MINIRHI_DECLARE_BINDING_SLOT_GETTER_INLINED_(Texture2DSlot, texture2d);
		MINIRHI_DECLARE_BINDING_SLOT_GETTER_INLINED_(Mat4Slot, mat4);
		MINIRHI_DECLARE_BINDING_SLOT_GETTER_INLINED_(UIntSlot, uint);
		MINIRHI_DECLARE_BINDING_SLOT_GETTER_INLINED_(FloatSlot, float);

	};

	template<typename... Slots>
	constexpr auto make_bindings(Slots&&... slots) noexcept {
		return BindingSet<Slots...> {
			.slots = std::make_tuple(std::forward<Slots>(slots)...)
		};
	}

	inline static constexpr auto kEmptyBindings = make_bindings();

	namespace detail {
		template<FixedString Code>
		consteval auto get_input_layout_tuple() {
			constexpr std::size_t N = ::minirhi::glsl::layout_count(Code);
			constexpr auto p = ::minirhi::glsl::parse_input_layout<N>(Code);

			return [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
				return std::make_tuple(CTString<FixedString<p[Ns].size()>(p[Ns])>{}...);
			}(std::make_index_sequence<N>{});
		}

		template<typename T>
		struct ToVtxAttr_;

#define MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(F, S) \
template<> \
struct ToVtxAttr_<CTString<FixedString(S)>> { \
	using Type = ::minirhi::VtxAttr<::minirhi::format::F>; \
};

		// TODO: Support other types
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(R32Float_t, "float");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RG32Float_t, "vec2");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RGB32Float_t, "vec3");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RGBA32Float_t, "vec4");

		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(R32UInt_t, "uint");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RG32UInt_t, "uvec2");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RGB32UInt_t, "uvec3");
		MINIRHI_DECLARE_VTX_ATTR_CONVERTER_(RGBA32UInt_t, "uvec4");

		template<typename T>
		struct ConvertStrToIL_;

		template<typename... Args>
		struct ConvertStrToIL_<const std::tuple<Args...>> {
			using Type = VtxAttrArr<typename ToVtxAttr_<Args>::Type...>;
		};

		template<FixedString Code>
		consteval auto generate_input_layout() {
			constexpr auto tuple = get_input_layout_tuple<Code>();
			return typename ConvertStrToIL_<decltype(tuple)>::Type {};
		}
	
		namespace tests {
			inline static constexpr auto kVS = FixedString(
		R"str(
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
layout (location = 2) in uint color;

out vec3 vert_color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vert_color = color;
}
)str");

			static_assert(
				std::same_as<
					decltype(generate_input_layout<kVS>()),
					::minirhi::VtxAttrArr<
						::minirhi::VtxAttr<::minirhi::format::RG32Float_t>,
						::minirhi::VtxAttr<::minirhi::format::RGB32Float_t>,
						::minirhi::VtxAttr<::minirhi::format::R32UInt_t>
					>
				>
			);
		}

		template<typename T>
		struct ConvertToBindingSet;

		template<typename... Slots>
		struct ConvertToBindingSet<const std::tuple<Slots...>> {
			using Type = BindingSet<Slots...>;
		};

		template<FixedString... Codes>
		consteval auto generate_binding_set_impl() {
			constexpr auto kArrays = std::make_tuple(::minirhi::glsl::parse_uniforms<::minirhi::glsl::uniform_count(Codes)>(Codes)...);

			constexpr auto kUniforms = [&]<std::size_t... Ns>([[maybe_unused]] std::index_sequence<Ns...>) {
				std::array<std::pair<std::string_view, std::string_view>, (::minirhi::glsl::uniform_count(Codes) + ...)> ret;
				std::size_t i = 0;
				((std::ranges::copy(
					std::begin(std::get<Ns>(kArrays)), 
					std::end(std::get<Ns>(kArrays)), 
					ret.begin() + i), 
					i += std::size(std::get<Ns>(kArrays))), ...);
				return ret;
			}(std::make_index_sequence<sizeof...(Codes)>{});

			constexpr auto kTuple = [&]<std::size_t... Ns>(std::index_sequence<Ns...>) {
				return std::make_tuple(
					Slot<
						CTString<FixedString<kUniforms[Ns].first.size()>(kUniforms[Ns].first)>,
						CTString<FixedString<kUniforms[Ns].second.size()>(kUniforms[Ns].second)>
					>{}...
				);
			}(std::make_index_sequence<kUniforms.size()>{});

			return typename ConvertToBindingSet<decltype(kTuple)>::Type{};
		}

		// kostil
		template<FixedString VS, FixedString FS>
		consteval auto generate_binding_set() {
			if constexpr (::minirhi::glsl::uniform_count(VS) == 0 && ::minirhi::glsl::uniform_count(FS) == 0){
				return kEmptyBindings;
			} else if constexpr (::minirhi::glsl::uniform_count(VS) == 0) {
				return generate_binding_set_impl<FS>();
			} else if constexpr (::minirhi::glsl::uniform_count(FS) == 0) {
				return generate_binding_set_impl<VS>();
			} else {
				return generate_binding_set_impl<VS, FS>();
			}
		}
	}

	struct BlendStateDesc {};

	enum class FrontFace {
		eClockWise = 0,
		eCounterClockWise,
	};

	enum class CullFaceMode {
		eFront = 0,
		eBack,
	};

	enum class PolygonMode {
		ePoint = 0,
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

	enum class DepthMask : u8 {
		eZero = 0,
		eAll,
	};

	enum class DepthFunc : u8 {
		eAlways = 0,
		eNever,
		eEq,
		eLe,
		eGr,
		eGr_Eq,
		eLe_Eq,
		eNot_Eq,
	};

	struct DepthStencilDesc {
		bool enable_depth = false;
		DepthMask depth_mask = DepthMask::eAll;
		DepthFunc depth_func = DepthFunc::eLe;
	};

	template<typename, typename>
	struct GraphicsPipelineDesc;

	namespace detail {
		union GraphicsPipelineRaw {
			u64 dummy_ = 0;
			struct {
				u32 topology : 2;
				// depth stencil
				u32 enable_depth : 1;
				u32 depth_mask : 1;
				u32 depth_fn : 3;
				// rasterizer
				u32 front_face : 1;
				u32 cull_mode_enabled : 1;
				u32 line_smooth_enabled : 1;
				u32 cull_mode : 1;
				u32 polygon_mode : 2;
				
				u32 padding : 19;
				u32 program : 32;
			} state;
		};
		static_assert(sizeof(GraphicsPipelineRaw) == sizeof(u64));
	}

	template<typename Attrs, typename BS>
	struct GraphicsPipeline {
		detail::GraphicsPipelineRaw raw;

		explicit  GraphicsPipeline() noexcept = default;

		explicit GraphicsPipeline(const GraphicsPipelineDesc<Attrs, BS>& desc, u32 program) noexcept {
			raw.state.program = program;
			raw.state.topology = u32(desc.topology);
			raw.state.enable_depth = u32(desc.depth_stencil.enable_depth);
			raw.state.depth_mask = u32(desc.depth_stencil.depth_mask);
			raw.state.depth_fn = u32(desc.depth_stencil.depth_func);
			raw.state.front_face = u32(desc.rasterizer.front);
			raw.state.cull_mode_enabled = u32(desc.rasterizer.cull_mode_enabled);
			raw.state.line_smooth_enabled = u32(desc.rasterizer.line_smooth_enabled);
			raw.state.cull_mode = u32(desc.rasterizer.cull_mode);
			raw.state.polygon_mode = u32(desc.rasterizer.polygon_mode);
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator==(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return raw.dummy_ == other.raw.dummy_;
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator!=(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return !(*this == other);
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator<(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return raw.dummy_ < other.raw.dummy_;
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator>=(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return !(*this < other);
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator>(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return raw.dummy_ > other.raw.dummy_;
		}

		template<typename OtherAttrs, typename OtherBS>
		bool operator<=(GraphicsPipeline<OtherAttrs, OtherBS> other) const noexcept {
			return !(*this > other);
		}
	};

	template<typename Attrs, typename BS>
	struct GraphicsPipelineDesc {
		VtxShaderHandle vs{};
		FragShaderHandle fs{};
		PrimitiveTopologyType topology = PrimitiveTopologyType::eCount;
		DepthStencilDesc depth_stencil;
		RasterizerStateDesc rasterizer{};

		explicit constexpr GraphicsPipelineDesc() noexcept = default;

		explicit constexpr GraphicsPipelineDesc(
			VtxShaderHandle vertex_shader,
			FragShaderHandle fragment_shader,
			PrimitiveTopologyType topology_type,
			const DepthStencilDesc& ds_desc,
			const RasterizerStateDesc& rasterizer_state 
		) noexcept
			: vs(vertex_shader)
			, fs(fragment_shader)
			, topology(topology_type)
			, depth_stencil(ds_desc)
			, rasterizer(rasterizer_state)
		{}

		GraphicsPipeline<Attrs, BS> build(bool destroy_shaders = true) noexcept {
			u32 shader_program = ShaderCompiler::link_shaders(vs, fs);
			assert(shader_program != kShaderInvalidHandle);

			if (destroy_shaders) {
				ShaderCompiler::destroy_shaders(vs, fs);
			}

			return GraphicsPipeline(*this, shader_program);
		}
	};

	template<FixedString VS, FixedString FS>
	inline auto generate_graphics_pipeline_from_shaders(PrimitiveTopologyType topology, const DepthStencilDesc& depth_stencil = {}, const RasterizerStateDesc& rasterizer = RasterizerStateDesc{}) noexcept {
		GraphicsPipelineDesc<
			decltype(detail::generate_input_layout<VS>()),
			decltype(detail::generate_binding_set<VS, FS>())
		> pipeline {
			ShaderCompiler::compile_from_code<VtxShaderHandle>(VS),
			ShaderCompiler::compile_from_code<FragShaderHandle>(FS),
			topology,
			depth_stencil,
			rasterizer
		};
		return pipeline.build(true);
	}
}