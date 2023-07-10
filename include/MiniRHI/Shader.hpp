#pragma once
#include "MiniRHI/Shader.hpp"
#include <concepts>
#include <string_view>
#include <format>
#include <limits>
#include <span>
#include <array>

#include <Core/Core.hpp>

namespace minirhi
{
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
