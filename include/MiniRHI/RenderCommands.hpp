#pragma once
#include "Core/Core.hpp"
#include "MiniRHI/Shader.hpp"
#include <type_traits>
#ifndef ANDROID
#include <glew/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/PipelineState.hpp"
#include "PipelineState.hpp"
#include "Buffer.hpp"
#include "Texture.hpp"

#include <array>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp> // TODO: add glm
#include <limits>

namespace minirhi
{
	struct Viewport {
		size_t x;
		size_t y;
		size_t width;
		size_t height;

		Viewport() noexcept = default;

		explicit constexpr Viewport(size_t w, size_t h, size_t x_ = 0, size_t y_ = 0) noexcept
			: x(x_)
			, y(y_)
			, width(w)
			, height(h)
		{}

		Viewport& SetRect(size_t w, size_t h) noexcept {
			width = w;
			height = h;
			return *this;
		}

		Viewport& SetPos(size_t x_, size_t y_) noexcept {
			x = x_;
			y = y_;
			return *this;
		}
	};
	
	template<typename PipelineAttrs, TVtxElem VtxElem, typename BS, typename... Slots>
	struct DrawParams {
		Viewport viewport{};
		PipelineState<PipelineAttrs, BS> pipeline{};
		VertexBufferRC<VtxElem> vertex_buffer{};
		IndexBufferRC index_buffer{};
		BindingSet<Slots...> bindings{};
	};

	template<typename PipelineAttrs, TVtxElem VtxElem, typename BS, typename... Slots>
	auto make_draw_params(
		const Viewport& vp, 
		const PipelineState<PipelineAttrs, BS>& ps, 
		VertexBufferRC<VtxElem> vb,
		const BindingSet<Slots...>& bindings = kEmptyBindings
	) noexcept {
		return DrawParams<PipelineAttrs, VtxElem, BS, Slots...> {
			.viewport = vp,
			.pipeline = ps,
			.vertex_buffer = vb,
			.bindings = bindings,
		};
	}

	template<typename PipelineAttrs, typename VtxElem, typename BS, typename... Slots>
	auto make_draw_params_indexed(
		const Viewport& vp, 
		const PipelineState<PipelineAttrs, BS>& ps, 
		VertexBufferRC<VtxElem> vb, 
		IndexBufferRC ib,
		const BindingSet<Slots...>& bindings = kEmptyBindings
	) noexcept {
		return DrawParams<PipelineAttrs, VtxElem, BS, Slots...> {
			.viewport = vp,
			.pipeline = ps,
			.vertex_buffer = vb,
			.index_buffer = ib,
			.bindings = bindings,
		};
	}

	namespace detail {
		template<typename UserBS, typename PipelineBS>
		struct DoesUserBindingSetMatch;

		template<typename... UserSlots, typename... PipelineSlots>
		struct DoesUserBindingSetMatch<BindingSet<UserSlots...>, BindingSet<PipelineSlots...>> {
			static constexpr bool kValue = (SameAsAny<PipelineSlots, UserSlots...> && ...);
		};
	}

	class RenderCommands {
	private:
		u32 vao_ = std::numeric_limits<u32>::max();
		std::size_t bound_texture_count_ = 0;

	public:
		explicit RenderCommands() noexcept;

		void clear_color_buffer(f32 r, f32 g, f32 b, f32 a) noexcept;
		void clear_depth_buffer() noexcept;
		void clear_stencil_buffer() noexcept;
		void clear_buffer(GLbitfield bufferType, f32 r, f32 g, f32 b, f32 a) noexcept;

		template<typename PipelineAttrs, typename VtxElem, typename BS, typename... Slots>
		void draw(const DrawParams<PipelineAttrs, VtxElem, BS, Slots...>& params, size_t vertex_count, size_t offset) noexcept {
			draw_common_setup_(params);
			draw_internal_(params.pipeline.topology, vertex_count, offset);
		}
		
		template<typename PipelineAttrs, typename VtxElem, typename BS, typename... Slots>
		void draw_indexed(const DrawParams<PipelineAttrs, VtxElem, BS, Slots...>& params, size_t index_count, size_t offset) noexcept {
			draw_common_setup_(params);
			draw_indexed_internal_(params.pipeline.topology, params.index_buffer.get().handle, index_count, offset);
		}
	
	private:
		template<template<typename, typename> typename Slot, typename Type, typename Name>
		void set_binding_(u32 program, const Slot<Type, Name>& v) noexcept {
			static constexpr FixedString  kName = Name::kValue;
			if constexpr (std::same_as<Slot<Type, Name>, Texture2DSlot<kName>>) {
				set_texture2d_binding_impl_(program, std::string_view(kName), v.value);
				return;
			} 
			if constexpr (std::same_as<Slot<Type, Name>, UIntSlot<kName>>) {
				set_uint_binding_impl_(program, std::string_view(kName), v.value);
				return;
			} 
			if constexpr (std::same_as<Slot<Type, Name>, FloatSlot<kName>>) {
				set_float_binding_impl_(program, std::string_view(kName), v.value);
			}
		}

		void set_texture2d_binding_impl_(u32 program, std::string_view name, u32 texture) noexcept;
		void set_uint_binding_impl_(u32 program, std::string_view name, u32 value) noexcept;
		void set_float_binding_impl_(u32 program, std::string_view name, f32 value) noexcept;


		template<typename PipelineAttrs, typename VtxElem, typename BS, typename... Slots>
		void draw_common_setup_(const DrawParams<PipelineAttrs, VtxElem, BS, Slots...>& params) noexcept {
			static_assert(TVtxElem<VtxElem>, "Vertex buffer element type must satisfy TVtxElem concept!");
			static_assert(std::same_as<PipelineAttrs, MakeVertexAttributes<VtxElem>>, "Vertex buffer's vertex attributes does not match the pipeline's vertex attributes!");
			static_assert(detail::DoesUserBindingSetMatch<BindingSet<Slots...>, BS>::kValue, "User-defined BindingSet does not match the pipeline's binding set!");
		
			auto& vb = params.vertex_buffer;
			auto& ib = params.index_buffer;
			static constexpr auto attrs = get_vtx_attr_array(PipelineAttrs{});
			setup_pipeline_(
				attrs, 
				params.pipeline.rasterizer, 
				params.viewport, 
				!vb.is_empty() ? vb.get().handle : kBufferInvalidHandle, 
				!ib.is_empty() ? ib.get().handle : kBufferInvalidHandle,
				params.pipeline.shader_program
			);

			if constexpr (!BS::kIsEmpty) {
				std::apply(
					[&](auto&&... slots) {
						(set_binding_(params.pipeline.shader_program, slots), ...);
					},
					params.bindings.slots
				);
			}
		}

		void setup_pipeline_(std::span<const VtxAttrData> attribs, const RasterizerStateDesc& rasterizer, const Viewport& vp, u32 vb, u32 ib, u32 program) noexcept;
		void draw_internal_(PrimitiveTopologyType type, size_t vertex_count, size_t offset) noexcept;
		void draw_indexed_internal_(PrimitiveTopologyType type, u32 ib, size_t index_count, size_t offset) noexcept;

		void set_rasterizer_state_(const RasterizerStateDesc& rs) noexcept;
	};
}