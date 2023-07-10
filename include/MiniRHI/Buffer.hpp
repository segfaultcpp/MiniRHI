#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>
#include <span>
#include <limits>

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/RC.hpp"
#include "MiniRHI/TypeInference.hpp"

#include <Core/Core.hpp>

namespace minirhi {
	enum class BufferType {
		eVertex,
		eIndex,
		eConstant,
		eCount
	};

	template<typename T>
	concept TVtxElem = TIsVertex<T>;

	u32 get_buffer_type(BufferType bufferType) noexcept;

	namespace detail {
		template<BufferType Type, typename Elem>
		struct BufferConstraints {
			static constexpr bool kSatisfied = false;
		};

		template<typename Elem>
		struct BufferConstraints<BufferType::eVertex, Elem> {
			static constexpr bool kSatisfied = TVtxElem<Elem>;
			static_assert(kSatisfied, "For minirhi::BufferDesc<Type, Elem> with [Type = BufferType::eVertex] Elem must satisfy minirhi::TVtxElem constraint!");
		};

		template<typename Elem>
		struct BufferConstraints<BufferType::eIndex, Elem> {
			static constexpr bool kSatisfied = std::same_as<Elem, u32>;
			static_assert(kSatisfied, "For minirhi::BufferDesc<Type, Elem> with [Type = BufferType::eIndex] Elem must be u32 integral type!");
		};

		template<typename Elem>
		struct BufferConstraints<BufferType::eConstant, Elem> {
			static constexpr bool kSatisfied = std::same_as<Elem, u8>;
			static_assert(kSatisfied, "For minirhi::BufferDesc<Type, Elem> with [Type = BufferType::eConstant] Elem must be u8 integral type!");
		};
	}

	template<BufferType Type, typename Elem>
	struct BufferDesc {
		static_assert(detail::BufferConstraints<Type, Elem>::kSatisfied, "Error! minirhi::BufferDesc constraints are not satisfied!");

		static constexpr BufferType kType = Type;
		std::span<const Elem> initial_data;

		explicit BufferDesc() noexcept
			: initial_data()
		{}

		explicit BufferDesc(std::span<const Elem> data)  noexcept
			: initial_data(data)
		{}

		[[nodiscard, gnu::always_inline]]
		std::size_t size_bytes() const noexcept {
			return initial_data.size_bytes();
		}

		[[nodiscard, gnu::always_inline]]
		std::size_t element_count() const noexcept {
			return initial_data.size();
		}

		[[nodiscard, gnu::always_inline]]
		const Elem* data() const noexcept {
			return initial_data.data();
		}
	};

	namespace detail {
		u32 create_buffer_(BufferType type, std::size_t size_in_bytes, const void* data) noexcept;
		void destroy_buffer_(u32& handle) noexcept;
	}

	inline static constexpr u32 kBufferInvalidHandle = std::numeric_limits<u32>::max();

	template<BufferType Type, typename Elem>
	struct BufferStorage {
		u32 handle = kBufferInvalidHandle;
		BufferDesc<Type, Elem> desc;

		static void destroy(BufferStorage& buf) noexcept {
			detail::destroy_buffer_(buf.handle);
		}

		BufferStorage() noexcept = default;
		explicit BufferStorage(const BufferDesc<Type, Elem> buffer_desc) noexcept
		    : handle(detail::create_buffer_(Type, buffer_desc.size_bytes(), std::bit_cast<const void*>(buffer_desc.data())))
			, desc(buffer_desc)
		{}	
	};

	template<TVtxElem Elem>
	using VertexBufferDesc = BufferDesc<BufferType::eVertex, Elem>;
	using IndexBufferDesc = BufferDesc<BufferType::eIndex, u32>;
	using ConstantBufferDesc = BufferDesc<BufferType::eConstant, u8>;

	template<TVtxElem Elem>
	struct VertexBuffer final : BufferStorage<BufferType::eVertex, Elem> {
		explicit VertexBuffer() noexcept = default;

		explicit VertexBuffer(std::span<const Elem> vertices) noexcept 
			: BufferStorage<BufferType::eVertex, Elem>{ VertexBufferDesc<Elem>{ vertices } }
		{}
	};

	struct IndexBuffer final : BufferStorage<BufferType::eIndex, u32> {
		explicit IndexBuffer() noexcept = default;

		explicit IndexBuffer(std::span<const u32> indices) noexcept 
			: BufferStorage<BufferType::eIndex, u32>{ IndexBufferDesc{ indices } }
		{}
	};

	struct ConstantBuffer final : BufferStorage<BufferType::eConstant, u8> {
		explicit ConstantBuffer() noexcept = default;

		explicit ConstantBuffer(std::span<const u8> constants) noexcept 
			: BufferStorage<BufferType::eConstant, u8>{ ConstantBufferDesc{ constants } }
		{}
	};

	template<BufferType Type, typename Elem>
	using BufferRC = RC<BufferStorage<Type, Elem>>;

	template<TVtxElem Elem>
	using VertexBufferRC = RC<VertexBuffer<Elem>>;
	using IndexBufferRC = RC<IndexBuffer>;
	using ConstantBufferRC = RC<ConstantBuffer>;

	template<BufferType Type, typename Elem>
	[[nodiscard]]
	BufferRC<Type, Elem> make_buffer_rc(const BufferDesc<Type, Elem> desc) noexcept {
		return BufferRC<Type, Elem>{ desc };
	}

	template<TVtxElem Elem>
	[[nodiscard]]
	VertexBufferRC<Elem> make_vertex_buffer_rc(std::span<const Elem> vertices) noexcept {
		return VertexBufferRC<Elem>{ vertices };
	}

	[[nodiscard]]
	inline IndexBufferRC make_index_buffer_rc(std::span<const u32> indices) noexcept {
		return IndexBufferRC{ indices };
	}

	[[nodiscard]]
	inline ConstantBufferRC make_constant_buffer_rc(std::span<const u8> constants) noexcept {
		return ConstantBufferRC{ constants };
	}
}
