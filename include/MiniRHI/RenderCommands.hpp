#pragma once
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

	enum class ItemType {
		eConstant,
		eTexture,
	};

	struct BindingSetItem {
		u32 location;
		u32 handle;
	};

	struct BindingSet {
		std::array<BindingSetItem, 16> textures;
		size_t bound_texture_count = 0;

		BindingSet& PushTexture(u32 location, u32 handle) noexcept
		{
			// TODO: BoundTextureCount > 16
			textures[bound_texture_count++] = BindingSetItem{ location, handle };
			return *this;
		}
	};
	
	template<typename PipelineAttrs, TVtxElem VtxElem>
	struct DrawParams {
		Viewport viewport{};
		PipelineState<PipelineAttrs> pipeline{};
		VertexBufferRC<VtxElem> vertex_buffer{};
		IndexBufferRC index_buffer{};
		BindingSet bindings{};
	};

	template<typename PipelineAttrs, TVtxElem VtxElem>
	auto make_draw_params(const Viewport& vp, const PipelineState<PipelineAttrs>& ps, VertexBufferRC<VtxElem> vb) noexcept {
		return DrawParams<PipelineAttrs, VtxElem> {
			.viewport = vp,
			.pipeline = ps,
			.vertex_buffer = vb,
		};
	}

	template<typename PipelineAttrs, typename VtxElem>
	auto make_draw_params_indexed(
		const Viewport& vp, 
		const PipelineState<PipelineAttrs>& ps, 
		VertexBufferRC<VtxElem> vb, 
		IndexBufferRC ib
	) noexcept {
		return DrawParams<PipelineAttrs, VtxElem> {
			.viewport = vp,
			.pipeline = ps,
			.vertex_buffer = vb,
			.index_buffer = ib,
		};
	}

	class RenderCommands {
	private:
		u32 vao_ = std::numeric_limits<u32>::max();

	public:
		explicit RenderCommands() noexcept;

		void clear_color_buffer(f32 r, f32 g, f32 b, f32 a) noexcept;
		void clear_depth_buffer() noexcept;
		void clear_stencil_buffer() noexcept;
		void clear_buffer(GLbitfield bufferType, f32 r, f32 g, f32 b, f32 a) noexcept;

		// TODO:
		// template<typename ConstantType>
		// void PushConstant(ShaderType shaderType, std::string_view name, ConstantType&& value) noexcept
		// {
		// 	u32 selectedShaderProgram = (shaderType == ShaderType::eVertex) ? _vertexShaderProgram : _fragmentShaderProgram;
		// 	assert(selectedShaderProgram != (u32)(~0) && "Invalid shader program. Set pipeline state first.");

		// 	u32 uniformLocation = glGetUniformLocation(selectedShaderProgram, name.data());
		// 	assert(uniformLocation != -1 && "Unknowed uniform name.");

		// 	PushConstant(selectedShaderProgram, uniformLocation, std::forward<ConstantType>(value));
		// }

		void PushConstant(u32 program, u32 location, TextureRC value) noexcept;
		void PushConstant(u32 program, u32 location, u32 value) noexcept;
		void PushConstant(u32 program, u32 location, f32 value) noexcept;
		void PushConstant(u32 program, u32 location, f32 x, f32 y) noexcept;
		void PushConstant(u32 program, u32 location, const glm::vec3& v) noexcept;
		void PushConstant(u32 program, u32 location, const glm::vec4& v) noexcept;
		void PushConstant(u32 program, u32 location, const glm::mat3x3& m) noexcept;
		void PushConstant(u32 program, u32 location, const glm::mat4x4& m) noexcept;

		template<typename PipelineAttrs, typename VtxElem>
		void draw(const DrawParams<PipelineAttrs, VtxElem>& params, size_t vertex_count, size_t offset) noexcept {
			static_assert(TVtxElem<VtxElem>, "Vertex buffer element type must satisfy TVtxElem concept!");
			static_assert(std::same_as<PipelineAttrs, MakeVertexAttributes<VtxElem>>, "Vertex buffer's vertex attributes does not match the pipeline's vertex attributes!");

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

			draw_internal_(params.pipeline.topology, vertex_count, offset);
		}
		
		template<typename PipelineAttrs, typename VtxElem>
		void draw_indexed(const DrawParams<PipelineAttrs, VtxElem>& params, size_t index_count, size_t offset) noexcept {
			static_assert(TVtxElem<VtxElem>, "Vertex buffer element type must satisfy TVtxElem concept!");
			static_assert(std::same_as<PipelineAttrs, MakeVertexAttributes<VtxElem>>, "Vertex buffer's vertex attributes does not match the pipeline's vertex attributes!");
		
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

			draw_indexed_internal_(params.pipeline.topology, params.index_buffer.get().handle, index_count, offset);
		}
	
	private:
		void setup_pipeline_(std::span<const VtxAttrData> attribs, const RasterizerStateDesc& rasterizer, const Viewport& vp, u32 vb, u32 ib, u32 program) noexcept;
		void draw_internal_(PrimitiveTopologyType type, size_t vertex_count, size_t offset) noexcept;
		void draw_indexed_internal_(PrimitiveTopologyType type, u32 ib, size_t index_count, size_t offset) noexcept;

		void set_rasterizer_state_(const RasterizerStateDesc& rs) noexcept;
	};
}