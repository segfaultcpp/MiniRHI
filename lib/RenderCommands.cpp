#include "MiniRHI/RenderCommands.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RC.hpp"

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

	RenderCommands::RenderCommands() noexcept {
		glGenVertexArrays(1, &vao_); // OpenGL requires VAO for drawing
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

	void RenderCommands::setup_pipeline_(std::span<const VtxAttrData> attribs, const RasterizerStateDesc& rasterizer, const Viewport& vp, u32 vb, u32 ib, u32 program) noexcept {
		glViewport(GLint(vp.x), GLint(vp.y), GLint(vp.width), GLint(vp.height));

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

	void RenderCommands::draw_internal_(PrimitiveTopologyType type, size_t vertex_count, size_t offset) noexcept {
		glBindVertexArray(vao_);
		glDrawArrays(convert_topology_type(type), offset, vertex_count);
		glBindVertexArray(0);
	}

	void RenderCommands::draw_indexed_internal_(PrimitiveTopologyType type, u32 ib, size_t index_count, size_t offset) noexcept {
		glBindVertexArray(vao_);
		glDrawElements(convert_topology_type(type), index_count, GL_UNSIGNED_INT, (void*)offset);
		glBindVertexArray(0);
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

	void RenderCommands::PushConstant(u32 program, u32 location, u32 value) noexcept
	{
		glProgramUniform1ui(program, location, value);
	}

	void RenderCommands::PushConstant(u32 program, u32 location, f32 value) noexcept
	{
		glProgramUniform1f(program, location, value);
	}

	void RenderCommands::PushConstant(u32 program, u32 location, f32 x, f32 y) noexcept
	{
		glProgramUniform2f(program, location, x, y);
	}

	void RenderCommands::PushConstant(u32 program, u32 location, const glm::vec3& v) noexcept
	{
		glProgramUniform3f(program, location, v.x, v.y, v.z);
	}

	void RenderCommands::PushConstant(u32 program, u32 location, const glm::vec4& v) noexcept
	{
		glProgramUniform4f(program, location, v.x, v.y, v.z, v.w);
	}

	void RenderCommands::PushConstant(u32 program, u32 location, const glm::mat3x3& m) noexcept
	{
		glProgramUniformMatrix3fv(program, location, 1, GL_FALSE, glm::value_ptr(m));
	}

	void RenderCommands::PushConstant(u32 program, u32 location, const glm::mat4x4& m) noexcept
	{
		glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, glm::value_ptr(m));
	}

	void RenderCommands::PushConstant(u32 program, u32 location, TextureRC value) noexcept
	{
		glProgramUniform1ui(program, location, value.get().handle);
	}
}
