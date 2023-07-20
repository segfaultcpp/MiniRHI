#include "MiniRHI/RenderCommands.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RC.hpp"

#include <limits>
#include <unordered_map>
#include <cassert>

#include <Core/Core.hpp>

#include <glm/gtc/type_ptr.hpp> // TODO: add glm

namespace minirhi
{
	static GLenum convert_topology_type(PrimitiveTopologyType type) noexcept {
		switch (type) {
		case PrimitiveTopologyType::ePoint: return GL_POINTS;
		case PrimitiveTopologyType::eLine: return GL_LINES;
		case PrimitiveTopologyType::eTriangle: return GL_TRIANGLES;
		default: return 0;
		}
	}

	static GLenum convert_polygon_mode(PolygonMode mode) noexcept {
		switch (mode) {
#ifndef ANDROID
		case PolygonMode::ePoint: return GL_POINT;
		case PolygonMode::eLine: return GL_LINE;
		case PolygonMode::eFill: return GL_FILL;
#endif
		default:
			return 0;
		}
	}

	static GLenum convert_front_face(FrontFace front) noexcept {
		switch (front) {
		case FrontFace::eClockWise: return GL_CW;
		case FrontFace::eCounterClockWise: return GL_CCW;
		}
		return 0;
	}

	static GLenum convert_cull_mode(CullFaceMode mode) noexcept {
		switch (mode) {
		case CullFaceMode::eBack: return GL_BACK;
		case CullFaceMode::eFront: return GL_FRONT;
		}
	}

	static GLenum convert_depth_func(DepthFunc func) noexcept {
		switch (func) {
		case DepthFunc::eAlways: return GL_ALWAYS;
		case DepthFunc::eNever: return GL_NEVER;
		case DepthFunc::eEq: return GL_EQUAL;
		case DepthFunc::eGr: return GL_GREATER;
		case DepthFunc::eGr_Eq: return GL_GEQUAL;
		case DepthFunc::eLe: return GL_LESS;
		case DepthFunc::eLe_Eq: return GL_LEQUAL;
		case DepthFunc::eNot_Eq: return GL_NOTEQUAL;
		}
		// TODO: Warn about unknown function
		return GL_LESS;
	}

	RenderCommands make_render_commands() noexcept {
		u32 vao = 0;
		glGenVertexArrays(1, &vao); // OpenGL requires VAO for drawing
		return RenderCommands(vao);
	}

	RenderCommands::~RenderCommands() noexcept {
		if (vao_ != std::numeric_limits<u32>::max()) {
			glDeleteVertexArrays(1, &vao_);
		}
	}

	void RenderCommands::clear_color_buffer(f32 r, f32 g, f32 b, f32 a) noexcept {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void RenderCommands::clear_depth_buffer() noexcept {
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void RenderCommands::clear_stencil_buffer() noexcept {
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	void RenderCommands::clear_buffer(GLbitfield buffer_type, f32 r, f32 g, f32 b, f32 a) noexcept {
		glClearColor(r, g, b, a);
		glClear(buffer_type);
	}

	void RenderCommands::setup_pipeline_(std::span<const VtxAttrData> attribs, const DepthStencilDesc& depth_stencil, const RasterizerStateDesc& rasterizer, const Viewport& vp, u32 vb, u32 ib, u32 program) noexcept {
		assert(vao_ != std::numeric_limits<u32>::max() && "Use of invalid render commands!");
		
		glViewport(GLint(vp.x), GLint(vp.y), GLint(vp.width), GLint(vp.height));

		if (depth_stencil.enable_depth) {
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GLboolean(depth_stencil.depth_mask == DepthMask::eAll));
			glDepthFunc(convert_depth_func(depth_stencil.depth_func));
		}

		glUseProgram(program);
		if (vb != kBufferInvalidHandle) {
			glBindVertexArray(vao_);
			glBindBuffer(GL_ARRAY_BUFFER, vb);

			if (ib != kBufferInvalidHandle) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
			}
		}

		std::size_t i = 0;
		for (auto[format, size, offset, stride] : attribs) {
			glVertexAttribPointer(
				i, 
				GLint(get_component_count(format)),
				get_format_type(format),
				GL_FALSE,
				GLsizei(stride),
				std::bit_cast<void*>(offset)
			);
			glEnableVertexAttribArray(i);
			i++;
		}

		if (vb != kBufferInvalidHandle) {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			if (ib != kBufferInvalidHandle) {
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
		}
		set_rasterizer_state_(rasterizer);
	}

	void RenderCommands::set_rasterizer_state_(const RasterizerStateDesc& rs) noexcept {
		if (rs.cull_mode_enabled) {
			glEnable(GL_CULL_FACE);
			glCullFace(convert_cull_mode(rs.cull_mode));
		} else {
			glDisable(GL_CULL_FACE);
		}
		glFrontFace(convert_front_face(rs.front));


		glLineWidth(rs.line_width);
#ifndef ANDROID
		glPolygonMode(GL_FRONT_AND_BACK, convert_polygon_mode(rs.polygon_mode));
		if (rs.line_smooth_enabled) {
			glEnable(GL_LINE_SMOOTH);
		} else {
			glDisable(GL_LINE_SMOOTH);
		}
#endif
	}

	void RenderCommands::set_texture2d_binding_impl_(u32 program, std::string_view name, u32 texture) noexcept {
		glUniform1i(glGetUniformLocation(program, name.data()), GLint(bound_texture_count_));
		glActiveTexture(GL_TEXTURE0 + bound_texture_count_);
		glBindTexture(GL_TEXTURE_2D, texture);
		bound_texture_count_++;
	}

	void RenderCommands::set_uint_binding_impl_(u32 program, std::string_view name, u32 value) noexcept {
		glProgramUniform1ui(program, glGetUniformLocation(program, name.data()), value);
	}

	void RenderCommands::set_float_binding_impl_(u32 program, std::string_view name, f32 value) noexcept {
		glProgramUniform1f(program, glGetUniformLocation(program, name.data()), value);
	}	

	void RenderCommands::set_mat4_binding_impl_(u32 program, std::string_view name, const glm::mat4& value) noexcept {
		glUniformMatrix4fv(glGetUniformLocation(program, name.data()), 1, GL_FALSE, glm::value_ptr(value));
	}

	void RenderCommands::draw_internal_(PrimitiveTopologyType type, size_t vertex_count, size_t offset) noexcept {
		glBindVertexArray(vao_);
		glDrawArrays(convert_topology_type(type), offset, vertex_count);
	}

	void RenderCommands::draw_indexed_internal_(PrimitiveTopologyType type, u32 ib, size_t index_count, size_t offset) noexcept {
		glBindVertexArray(vao_);
		glDrawElements(convert_topology_type(type), index_count, GL_UNSIGNED_INT, (void*)offset);
	}

	void RenderCommands::draw_end_() noexcept {
		glBindVertexArray(0);
		bound_texture_count_ = 0;

		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
	}

	// TODO:
	// void RenderCommands::SetBindingSet(const BindingSet& bs) noexcept {
	// 	for (size_t i = 0; i < bs.BoundTextureCount; ++i)
	// 	{
	// 		BindingSetItem tex = bs.Textures[i];
	// 		glActiveTexture(GL_TEXTURE0 + i);
	// 		glBindTexture(GL_TEXTURE_2D, tex.Handle);

	// 		// #TODO: Remove that
	// 		glUniform1i(tex.Handle, i);
	// 	}
	// }
}
