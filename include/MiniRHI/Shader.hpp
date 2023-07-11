#pragma once
#include "MiniRHI/Shader.hpp"
#include <concepts>
#include <string_view>
#include <format>
#include <limits>
#include <span>
#include <array>

#include <Core/Core.hpp>

namespace minirhi {
	namespace glsl {
		enum class TokenType {
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
				return Token{ "", TokenType::eEof };
			}

			static constexpr Token unknown() noexcept {
				return Token{ "", TokenType::eUnknown };
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

					std::string_view value(begin, current);
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

					std::string_view value(begin, current);
					return Token{ value, TokenType::eNum };
				}

				It begin = current;
				switch (*current) {
				case '(': ++current; return Token{ std::string_view(begin, current), TokenType::eL_Paren };
				case ')': ++current; return Token{ std::string_view(begin, current), TokenType::eR_Paren };
				case '=': ++current; return Token{ std::string_view(begin, current), TokenType::eEqSign };
				case ';': ++current; return Token{ std::string_view(begin, current), TokenType::eSemicolon };
				}
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

		consteval std::size_t layout_count(std::string_view code) {
			std::size_t count = 0;
			for (std::size_t n = code.find("layout", 0); n != std::string_view::npos; n = code.find("layout", n)) {
				n += 6;
				++count;
			}
			return count;
		}

		template<std::size_t N>
		consteval auto parse_input_layout(std::string_view code) {
			const std::size_t off = code.find("layout");
			const std::string_view code_off(std::next(code.begin(), off), code.end());

			Lexer lexer{ code_off };
			Token token{};
			std::array<std::string_view, N> attr_types;
			std::size_t i = 0;
			
			while (((token = lexer.next()) != Token::end_of_file()) && (token != Token::unknown())) {
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
