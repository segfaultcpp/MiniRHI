#pragma once
#include "MiniRHI/Shader.hpp"
#include <concepts>
#include <string_view>
#include <limits>
#include <span>
#include <array>
#include <compare>

#include <Core/Core.hpp>

namespace minirhi {
	namespace glsl {
		struct TypeNames {
			// Scalars
			static constexpr const char kUInt[] = "uint";
			static constexpr const char kFloat[] = "float";

			// Samplers
			static constexpr const char kSampler2D[] = "sampler2D";

			// Vectors
			static constexpr const char kVec2[] = "vec2";
			static constexpr const char kVec3[] = "vec3";
			static constexpr const char kVec4[] = "vec4";

			// Matrices
			static constexpr const char kMat2[] = "mat2";
			static constexpr const char kMat3[] = "mat3";
			static constexpr const char kMat4[] = "mat4";
		};

		enum class TokenType {
			eKW_Uniform,
			eKW_Layout,
			eKW_Location,
			eKW_In,
			eL_Paren,
			eR_Paren,
			eEqSign,
			eSemicolon,
			eNum,
			eIdent,
			eEof,
			eUnknown,
		};

		struct Token {
			std::string_view value;
			TokenType type;

			static constexpr Token end_of_file() noexcept {
				return Token{ "__eof", TokenType::eEof };
			}

			static constexpr Token unknown() noexcept {
				return Token{ "__unknown_token", TokenType::eUnknown };
			}

			constexpr bool operator==(const Token& other) const noexcept {
				return type == other.type;
			}

			constexpr bool operator!=(const Token& other) const noexcept {
				return type != other.type;
			}
		};

		struct Lexer {
			using It = std::string_view::const_iterator;
			It end;
			It current;

			static constexpr std::string_view make_sv(It begin, It end)
			{
#ifdef ANDROID
				return std::string_view(begin, (end-begin));
#else
				return std::string_view(begin, end);
#endif
			}

			explicit constexpr Lexer(std::string_view c) noexcept
				: end(c.end())
				, current(c.begin())
			{}

			constexpr Token next() noexcept {
				while (current != end && is_whitespace_(*current)) {
					++current;
				}
				
				if (current == end) {
					return Token::end_of_file();
				}

				if (is_letter_(*current)) {
					It begin = current++;

					while (satisfies_rules_for_ident_(*current)) {
						++current;
					}

					std::string_view value = make_sv(begin, current);
					if (value == "uniform") {
						return Token{ value, TokenType::eKW_Uniform };
					}
					if (value == "layout") {
						return Token{ value, TokenType::eKW_Layout };
					}
					if (value == "location") {
						return Token{ value, TokenType::eKW_Location };
					}
					if (value == "in") {
						return Token{ value, TokenType::eKW_In };
					}
					if (is_ident_(value)) {
						return Token{ value, TokenType::eIdent };
					}
					return Token::unknown();
				}
				if (is_digit_(*current)) {
					It begin = current++;

					while (is_digit_(*current)) {
						++current;
					}

					std::string_view value = make_sv(begin, current);
					return Token{ value, TokenType::eNum };
				}

				It begin = current;
				switch (*current) {
				case '(': ++current; return Token{ make_sv(begin, current), TokenType::eL_Paren };
				case ')': ++current; return Token{ make_sv(begin, current), TokenType::eR_Paren };
				case '=': ++current; return Token{ make_sv(begin, current), TokenType::eEqSign };
				case ';': ++current; return Token{ make_sv(begin, current), TokenType::eSemicolon };
				}
				++current;
				return Token::unknown();
			}

		private:
			[[gnu::always_inline, nodiscard]]
			static constexpr bool is_whitespace_(char c) noexcept {
				return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\v' || c == '\r';
			}
			
			[[gnu::always_inline, nodiscard]]
			static constexpr bool is_digit_(char c) noexcept {
				return c >= '0' && c <= '9';
			}

			[[gnu::always_inline, nodiscard]]
			static constexpr bool is_letter_(char c) noexcept {
				return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
			}

			[[gnu::always_inline, nodiscard]]
			static constexpr bool satisfies_rules_for_ident_(char c) noexcept {
				return is_letter_(c) || is_digit_(c) || c == '_';
			}

			[[gnu::always_inline, nodiscard]]
			static constexpr bool is_ident_(std::string_view value) noexcept {
				for (char c : value) {
					if (not satisfies_rules_for_ident_(c)) {
						return false;
					}
				}
				return true;
			}
		};

		consteval std::size_t name_count(std::string_view code, std::string_view name) {
			std::size_t count = 0;
			for (std::size_t n = code.find(name, 0); n != std::string_view::npos; n = code.find(name, n)) {
				n += name.size();
				++count;
			}
			return count;
		}

		consteval std::size_t layout_count(std::string_view code) {
			return name_count(code, "layout");
		}

		consteval std::size_t uniform_count(std::string_view code) {
			return name_count(code, "uniform");
		}

		// grammar rule: 'layout' '(' 'location' '=' num ')' ident ident ';'
		template<std::size_t N>
		consteval auto parse_input_layout(std::string_view code) {
			if constexpr (N == 0) {
				return;
			}

			const std::size_t off = code.find("layout");
			const std::string_view code_off = Lexer::make_sv(std::next(code.begin(), off), code.end());

			Lexer lexer{ code_off };
			Token token{};
			std::array<std::string_view, N> attr_types;
			std::size_t i = 0;
			
			while (((token = lexer.next()) != Token::end_of_file()) && (i < N)) {
				if (token.type == TokenType::eKW_Layout) {
					if (token = lexer.next(); token.type != TokenType::eL_Paren) {
						throw "Unexpected token. Expected '('.";
					}
					if (token = lexer.next(); token.type != TokenType::eKW_Location) {
						throw "Unexpected token. Expected 'location'.";
					}
					if (token = lexer.next(); token.type != TokenType::eEqSign) {
						throw "Unexpected token. Expected '='.";
					}
					if (token = lexer.next(); token.type != TokenType::eNum) {
						throw "Unexpected token. Expected location index.";
					}
					if (token = lexer.next(); token.type != TokenType::eR_Paren) {
						throw "Unexpected token. Expected ')'.";
					}
					if (token = lexer.next(); token.type != TokenType::eKW_In) {
						throw "Unexpected token. Expected 'in'.";
					}
					if (token = lexer.next(); token.type != TokenType::eIdent) {
						throw "Unexpected token. Expected type identifier.";
					}
					attr_types[i++] = token.value;

					if (token = lexer.next(); token.type != TokenType::eIdent) {
						throw "Unexpected token. Expected identifier.";
					}
					if (token = lexer.next(); token.type != TokenType::eSemicolon) {
						throw "Unexpected token. Expected type identified.";
					}
				}
			}

			return attr_types;
		}

		// grammar rule: 'uniform' ident ident ';'
		template<std::size_t N>
 		consteval auto parse_uniforms(std::string_view code) {
			const std::size_t off = code.find("uniform");
			const std::string_view code_off = Lexer::make_sv(std::next(code.begin(), off), code.end());

			Lexer lexer{ code_off };
			Token token{};
			std::array<std::pair<std::string_view, std::string_view>, N> uniforms;
			std::size_t i = 0;

			while (((token = lexer.next()) != Token::end_of_file()) && (i < N)) {
				if (token.type == TokenType::eKW_Uniform) {
					if (token = lexer.next(); token.type != TokenType::eIdent) {
						throw "Unexpected token. Expected identifier.";
					}
					auto type_name = token.value;
					if (token = lexer.next(); token.type != TokenType::eIdent) {
						throw "Unexpected token. Expected identifier.";
					}
					auto obj_name = token.value;
					uniforms[i++] = std::make_pair(type_name, obj_name);
				}
			}
			return uniforms;
		}

		namespace tests {
			using namespace std::string_view_literals;

			inline static constexpr FixedString kShader = R"str(
#version 330 core
layout (location = 0) in vec2 position;
// comment 
// another one
/*
* and this one
*/
layout (location = 1) in vec3 color;
uniform uint baz;
layout (location = 2) in uint color;

out vec3 vert_color;

uniform vec2 foo;


out vec2 vert_pos;
uniform vec3 light_pos;
)str";

			static_assert(layout_count(kShader) == 3);
			static_assert(uniform_count(kShader) == 3);

			static_assert(
				parse_input_layout<layout_count(kShader)>(kShader) ==
				std::to_array({ "vec2"sv, "vec3"sv, "uint"sv })
			);

			static_assert(
				parse_uniforms<uniform_count(kShader)>(kShader) ==
				std::to_array({ 
					std::make_pair("uint"sv, "baz"sv),
					std::make_pair("vec2"sv, "foo"sv), 
					std::make_pair("vec3"sv, "light_pos"sv),
				})
			);
		}
	}

	enum class ShaderType {
		eVertex,
		eFragment,

		eCount,
	};

	inline static constexpr u32 kShaderInvalidHandle = std::numeric_limits<u32>::max();

	template<ShaderType Type>
	struct ShaderHandle {
		u32 handle;

		auto operator<=>(const ShaderHandle& other) const = default;
	};

	using VtxShaderHandle = ShaderHandle<ShaderType::eVertex>;
	using FragShaderHandle = ShaderHandle<ShaderType::eFragment>;

	namespace detail {
		VtxShaderHandle compile_vtx_shader_impl_(std::string_view code) noexcept;
		FragShaderHandle compile_frag_shader_impl_(std::string_view code) noexcept;
		void destroy_shader_impl_(u32 shader) noexcept;
	}

	class ShaderCompiler {
	public:
		template<typename ShType>
		[[nodiscard]]
		static ShType compile_from_code(std::string_view code) noexcept {
			if constexpr (std::same_as<ShType, VtxShaderHandle>) {
				return detail::compile_vtx_shader_impl_(code); 
			} else /*if (std::same_as<ShType, FragShaderHandle>)*/ {
				return detail::compile_frag_shader_impl_(code);
			}
		}

		static u32 link_shaders_span(std::span<u32> shaders) noexcept;

		template<typename... Shaders>
		static u32 link_shaders(Shaders... shaders) noexcept {
			auto shaders_arr = std::to_array({
				shaders.handle...
			});
			return link_shaders_span(shaders_arr);
		}

		template<typename... Shaders>
		static void destroy_shaders(Shaders... shaders) noexcept {
			(destroy_shader(shaders), ...);
		}

		template<ShaderType Type>
		static void destroy_shader(ShaderHandle<Type> shader) noexcept {
			detail::destroy_shader_impl_(shader.handle);
		}
	};

}
