#include "MiniRHI/CmdCtx.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RC.hpp"

#include <limits>
#include <unordered_map>
#include <cassert>

#include <Core/Core.hpp>

#include <glm/gtc/type_ptr.hpp> // TODO: add glm

namespace minirhi {
	extern u32 gDefaultVAO;

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

	u32 CmdCtx::create_vao_() noexcept {
		return gDefaultVAO;
	}

	void CmdCtx::setup_pipeline_(u32 vao, std::span<const VtxAttrData> attribs, detail::GraphicsPipelineRaw pipeline, const Viewport& vp) noexcept {
		glViewport(GLint(vp.x), GLint(vp.y), GLint(vp.width), GLint(vp.height));

		if (bool(pipeline.state.enable_depth)) {
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GLboolean(DepthMask(u32(pipeline.state.depth_mask)) == DepthMask::eAll));
			glDepthFunc(convert_depth_func(DepthFunc(u32(pipeline.state.depth_fn))));
		}

		glBindVertexArray(vao);
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

		glUseProgram(pipeline.state.program);
		
		if (bool(pipeline.state.cull_mode_enabled)) {
			glEnable(GL_CULL_FACE);
			glCullFace(convert_cull_mode(CullFaceMode(u32(pipeline.state.cull_mode))));
		} else {
			glDisable(GL_CULL_FACE);
		}
		glFrontFace(convert_front_face(FrontFace(u32(pipeline.state.front_face))));

#ifndef ANDROID
		glPolygonMode(GL_FRONT_AND_BACK, convert_polygon_mode(PolygonMode(u32(pipeline.state.polygon_mode))));
		if (bool(pipeline.state.line_smooth_enabled)) {
			glEnable(GL_LINE_SMOOTH);
		} else {
			glDisable(GL_LINE_SMOOTH);
		}
#endif
	}

	namespace detail {
		void clear_color_buffer_impl_(f32 r, f32 g, f32 b, f32 a) noexcept {
			glClearColor(r, g, b, a);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		void clear_depth_buffer_impl_() noexcept {
			glClear(GL_DEPTH_BUFFER_BIT);
		}

		void clear_stencil_buffer_impl_() noexcept {
			glClear(GL_STENCIL_BUFFER_BIT);
		}

		void unset_pipeline_impl_() noexcept {
			glBindVertexArray(0);

			glDisable(GL_CULL_FACE);
			glFrontFace(GL_CCW);

			glLineWidth(1.0);
	#ifndef ANDROID
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDisable(GL_LINE_SMOOTH);
	#endif

			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			glDisable(GL_DEPTH_TEST);
		}
	
		void draw_impl_(PrimitiveTopologyType topology, u32 vb, size_t vertex_count, size_t offset) noexcept {
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glDrawArrays(convert_topology_type(topology), GLint(offset), GLsizei(vertex_count));
		}

		void draw_indexed_impl_(PrimitiveTopologyType topology, u32 vb, u32 ib, size_t index_count, size_t offset) noexcept {
			glBindBuffer(GL_ARRAY_BUFFER, vb);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
			glDrawElements(convert_topology_type(topology), GLsizei(index_count), GL_UNSIGNED_INT, std::bit_cast<void*>(offset));
		}

		void set_texture2d_binding_impl_(u32 bound_texture_count, u32 program, std::string_view name, u32 texture) noexcept {
			glUniform1i(glGetUniformLocation(program, name.data()), GLint(bound_texture_count));
			glActiveTexture(GL_TEXTURE0 + bound_texture_count);
			glBindTexture(GL_TEXTURE_2D, texture);
		}

		void set_uint_binding_impl_(u32 program, std::string_view name, u32 value) noexcept {
			glProgramUniform1ui(program, glGetUniformLocation(program, name.data()), value);
		}

		void set_float_binding_impl_(u32 program, std::string_view name, f32 value) noexcept {
			glProgramUniform1f(program, glGetUniformLocation(program, name.data()), value);
		}	

		void set_mat4_binding_impl_(u32 program, std::string_view name, const glm::mat4& value) noexcept {
			glUniformMatrix4fv(glGetUniformLocation(program, name.data()), 1, GL_FALSE, glm::value_ptr(value));
		}
	}
}
