#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <limits>

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/RC.hpp"

#include <Core/Core.hpp>

namespace minirhi {
	enum class BufferType {
		eVertex,
		eIndex,
		eConstant,
		eCount
	};

	template<typename TElem>
	struct BufferDesc {
		BufferType type;
		std::span<const TElem> initial_data;

		BufferDesc() noexcept
			: type(BufferType::eCount)
			, initial_data()
		{}

		explicit BufferDesc(BufferType buffer_type, std::span<const TElem> data)  noexcept
			: type(buffer_type)
			, initial_data(data)
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
		const TElem* data() const noexcept {
			return initial_data.data();
		}
	};
	
	template<typename T, std::size_t N>
	BufferDesc(BufferType, std::span<T, N>) -> BufferDesc<std::remove_cvref_t<T>>;

	namespace detail {
		u32 create_buffer_(BufferType type, std::size_t size_in_bytes, const void* data) noexcept;
		void destroy_buffer_(u32& handle) noexcept;
	}

	template<typename T>
	concept TVtxElem = requires() {
		T::get_attrs();
	};

	static constexpr u32 kBufferInvalidHandle = std::numeric_limits<u32>::max();

	template<typename TElem>
	class Buffer {
	public:
		u32 handle = kBufferInvalidHandle;
		BufferDesc<TElem> desc;	

		using DestroyFn = void(Buffer&);
		static void destroy(Buffer& buf) noexcept {
			detail::destroy_buffer_(buf.handle);
		}

		Buffer() noexcept = default;
		explicit Buffer(const BufferDesc<TElem> buffer_desc) noexcept
		    : desc(buffer_desc)
			, handle(detail::create_buffer_(buffer_desc.type, buffer_desc.size_bytes(), std::bit_cast<const void*>(buffer_desc.data())))
		{}	
	};

	template<typename T>
	using BufferRC = RC<Buffer<T>, Buffer<T>::destroy>;

	template<typename T>
	BufferRC<T> make_buffer_rc(const BufferDesc<T> desc) noexcept {
		return BufferRC<T>{ desc };
	}
}
