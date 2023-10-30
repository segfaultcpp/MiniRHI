#include "MiniRHI/Shader.hpp"
#include <algorithm>

#ifndef ANDROID
#include <glew/glew.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif

#include <concepts>
#include <array>
#include <bit>

#include <iostream>

namespace minirhi {
	namespace detail {
		[[nodiscard]]
		static u32 compile_shader_internal_impl_(std::string_view code, u32 sh_type) noexcept {
			u32 shader = glCreateShader(GLenum(sh_type)); 
			auto* code_ptr = std::bit_cast<const GLchar*>(code.data());
			
			glShaderSource(shader, 1, &code_ptr, nullptr);
			glCompileShader(shader);

			GLint success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			
			if (!static_cast<bool>(success)) {
				std::array<char, 512> log{};
				std::fill_n(log.begin(), log.size(), 0);
				glGetShaderInfoLog(shader, log.size(), nullptr, log.data());

				std::string_view log_view(log.data(), log.size());
				std::cerr << "Error! Failed to compile shader! Reason: {}" << log_view << std::endl;

				return kShaderInvalidHandle;
			}

			return shader;
		}

		VtxShaderHandle compile_vtx_shader_impl_(std::string_view code) noexcept {
			return VtxShaderHandle{ compile_shader_internal_impl_(code, GL_VERTEX_SHADER) };
		}

		FragShaderHandle compile_frag_shader_impl_(std::string_view code) noexcept {
			return FragShaderHandle{ compile_shader_internal_impl_(code, GL_FRAGMENT_SHADER) };
		}

		void destroy_shader_impl_(u32 shader) noexcept {
			glDeleteShader(GLuint(shader));
		}
	}

	u32 ShaderCompiler::link_shaders_span(std::span<u32> shaders) noexcept {
		u32 program = glCreateProgram();

		for (u32 shader : shaders) {
			glAttachShader(program, shader);
		}
		glLinkProgram(program);

		GLint success = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (!static_cast<bool>(success)) {
			std::array<char, 512> log{};
			std::fill_n(log.begin(), log.size(), 0);
			glGetProgramInfoLog(program, log.size(), nullptr, log.data());

			std::string_view log_view(log.data(), log.size());
			std::cerr << "Error! Failed to link shaders! Reason: {}" << log_view << std::endl;

			return kShaderInvalidHandle;
		}
		
		return program;
	}
}