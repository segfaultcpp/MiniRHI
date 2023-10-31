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
#include <glm/vec4.hpp> 
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
	};

	namespace detail {
		template<typename UserBS, typename PipelineBS>
		struct DoesUserBindingSetMatch;

		template<typename... UserSlots, typename... PipelineSlots>
		struct DoesUserBindingSetMatch<BindingSet<UserSlots...>, BindingSet<PipelineSlots...>> {
			static constexpr bool kValue = (SameAsAny<PipelineSlots, UserSlots...> && ...);
		};
	
		static constexpr u32 kInvalidVAHandle = std::numeric_limits<u32>::max();

		void clear_color_buffer_impl_(f32 r, f32 g, f32 b, f32 a) noexcept;
		void clear_depth_buffer_impl_() noexcept;
		void clear_stencil_buffer_impl_() noexcept;

		void unset_pipeline_impl_() noexcept;
		void draw_impl_(PrimitiveTopologyType topology, u32 vb, size_t vertex_count, size_t offset) noexcept;
		void draw_indexed_impl_(PrimitiveTopologyType topology, u32 vb, u32 ib, size_t index_count, size_t offset) noexcept;
	
		void set_texture2d_binding_impl_(u32 bound_texture_count, u32 program, std::string_view name, u32 texture) noexcept;
		void set_uint_binding_impl_(u32 program, std::string_view name, u32 value) noexcept;
		void set_float_binding_impl_(u32 program, std::string_view name, f32 value) noexcept;
		void set_mat4_binding_impl_(u32 program, std::string_view name, const glm::mat4& value) noexcept;

		void borrow_context_() noexcept;
		void release_context_() noexcept;
	}

	template<typename Attrs, typename BS>
	class [[nodiscard]] DrawCtx {
		friend class CmdCtx;
	private:
		static constexpr auto kAttrs = get_vtx_attr_array(Attrs{});

		u32 vao_ = detail::kInvalidVAHandle;
		u32 program_ = kShaderInvalidHandle;
		PrimitiveTopologyType topology_ = PrimitiveTopologyType::eTriangle;

		DrawCtx(u32 vao, u32 program, PrimitiveTopologyType topology) noexcept 
			: vao_(vao)
			, program_(program)
			, topology_(topology)
		{}

	public:
		DrawCtx(const DrawCtx&) = delete;
		DrawCtx& operator=(const DrawCtx&) = delete;

		DrawCtx(DrawCtx&& rhs) noexcept 
			: vao_(rhs.vao_)
			, program_(rhs.program_)
			, topology_(rhs.topology_)
		{
			rhs.vao_ = detail::kInvalidVAHandle;
			rhs.program_ = kShaderInvalidHandle;
			rhs.topology_ = PrimitiveTopologyType::eCount;
		}

		DrawCtx& operator=(DrawCtx&& rhs) noexcept {
			if (this == &rhs) {
				return *this;
			}

			vao_ = rhs.vao_;
			program_ = rhs.program_;
			topology_ = rhs.topology_;

			rhs.vao_ = detail::kInvalidVAHandle;
			rhs.program_ = kShaderInvalidHandle;
			rhs.topology_ = PrimitiveTopologyType::eCount;

			return *this;
		}

		~DrawCtx() noexcept {
			finish();
		}

		template<typename... Slots>
		void set_bindings(const BindingSet<Slots...>& bs) const noexcept {
			static_assert(detail::DoesUserBindingSetMatch<BindingSet<Slots...>, BS>::kValue, "User-defined BindingSet does not match the pipeline's binding set!");
			u32 bound_texture_count = 0;
			(set_binding_(bs.template get_slot<Slots>(), bound_texture_count), ...);
		}

		template<TVtxElem Elem>
		void draw(VertexBufferRC<Elem> vb, size_t vertex_count, size_t offset) const noexcept {
			static_assert(std::same_as<Attrs, MakeVertexAttributes<Elem>>, "Vertex buffer's vertex attributes does not match the pipeline's vertex attributes!");
			detail::draw_impl_(topology_, vb.get().handle, vertex_count, offset);
		}

		template<TVtxElem Elem>
		void draw_indexed(VertexBufferRC<Elem> vb, IndexBufferRC ib, size_t index_count, size_t offset) const noexcept {
			static_assert(std::same_as<Attrs, MakeVertexAttributes<Elem>>, "Vertex buffer's vertex attributes does not match the pipeline's vertex attributes!");
			detail::draw_indexed_impl_(topology_, vb.get().handle, ib.get().handle, index_count, offset);
		}

		void finish() noexcept {
			if (vao_ != detail::kInvalidVAHandle) {
				vao_ = detail::kInvalidVAHandle;
				program_ = kShaderInvalidHandle;
				topology_ = PrimitiveTopologyType::eCount;

				detail::release_context_();
				detail::unset_pipeline_impl_();
			}
		}

		void clear_color_buffer(f32 r, f32 g, f32 b, f32 a) noexcept {
			detail::clear_color_buffer_impl_(r, g, b, a);
		}

		void clear_depth_buffer() noexcept {
			detail::clear_depth_buffer_impl_();
		}

		void clear_stencil_buffer() noexcept {
			detail::clear_stencil_buffer_impl_();
		}

	private:
		template<template<typename, typename> typename Slot, typename Type, typename Name>
		void set_binding_(const Slot<Type, Name>& v, u32& bound_texture_count) const noexcept {
			static constexpr FixedString  kName = Name::kValue;
			if constexpr (std::same_as<Slot<Type, Name>, Texture2DSlot<kName>>) {
				detail::set_texture2d_binding_impl_(bound_texture_count, program_, std::string_view(kName), v.value.get().handle);
				bound_texture_count++;
				return;
			} 
			if constexpr (std::same_as<Slot<Type, Name>, UIntSlot<kName>>) {
				detail::set_uint_binding_impl_(program_, std::string_view(kName), v.value);
				return;
			} 
			if constexpr (std::same_as<Slot<Type, Name>, FloatSlot<kName>>) {
				detail::set_float_binding_impl_(program_, std::string_view(kName), v.value);
			}
			if constexpr (std::same_as<Slot<Type, Name>, Mat4Slot<kName>>) {
				detail::set_mat4_binding_impl_(program_, std::string_view(kName), v.value);
			}
		}

	};

	class CmdCtx {
	public:
		template<typename Attrs, typename BS>
		[[nodiscard]]
		static DrawCtx<Attrs, BS> start_draw_context(const Viewport& vp, GraphicsPipeline<Attrs, BS> ps) noexcept {
			detail::borrow_context_();
			static constexpr auto kAttrs = get_vtx_attr_array(Attrs{});
			
			u32 vao = create_vao_();
			setup_pipeline_(vao, kAttrs, ps.raw, vp);

			return DrawCtx<Attrs, BS>(vao, u32(ps.raw.state.program), PrimitiveTopologyType(u32(ps.raw.state.topology)));
		}
	private:
		static void setup_pipeline_(u32 vao, std::span<const VtxAttrData> attribs, detail::GraphicsPipelineRaw pipeline, const Viewport& vp) noexcept;
		static u32 create_vao_() noexcept;
		void draw_internal_(PrimitiveTopologyType type, size_t vertex_count, size_t offset) noexcept;
	};
}