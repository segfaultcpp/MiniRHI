#pragma once
#include <concepts>
#include <cassert>

#include <Core/Core.hpp>
#include <type_traits>

namespace minirhi {
	template<typename Res, auto Destroy = Res::destroy>
		requires std::is_trivially_copyable_v<Res> && std::invocable<decltype(Destroy), Res&>
	class RC {
	private:
		size_t* ref_count_ = nullptr;
		Res _handle;

	public:
		explicit constexpr RC() noexcept = default;

		template<typename... Args> requires std::bool_constant<sizeof...(Args) != 0>::value
		explicit constexpr RC(Args&&... args) noexcept
			: ref_count_(new size_t(1u))
			, _handle(std::forward<Args>(args)...)
		{}

		constexpr RC(const RC& rhs) noexcept
			: ref_count_(rhs.ref_count_)
			, _handle(rhs._handle)
		{

			if (!std::is_constant_evaluated()) {
				assert(rhs.ref_count_ != nullptr && "Cannot copy invalid RC object.");
			}
			++(*ref_count_);
		}

		constexpr RC& operator=(const RC& rhs) noexcept {
			if (this == &rhs || ref_count_ == rhs.ref_count_) {
				return *this;
			}

			if (ref_count_ != nullptr) {
				--(*ref_count_);
			}
			
			if (!std::is_constant_evaluated()) {
				assert(rhs.ref_count_ != nullptr && "Cannot copy invalid RC object.");
			}

			ref_count_ = rhs.ref_count_;
			++(*ref_count_);

			_handle = rhs._handle;

			return *this;
		}

		constexpr RC(RC&& rhs) noexcept
			: ref_count_(rhs.ref_count_)
			, _handle(std::move(rhs._handle))
		{
			if (!std::is_constant_evaluated()) {
				assert(rhs.ref_count_ != nullptr && "Cannot move invalid RC object.");
			}
			rhs.ref_count_ = nullptr;
		}

		constexpr RC& operator=(RC&& rhs) noexcept {
			if (this == &rhs) {
				return *this;
			}
			if (!std::is_constant_evaluated()) {
				assert(rhs.ref_count_ != nullptr && "Cannot move invalid RC object.");
			}
			
			if (ref_count_ == rhs.ref_count_) {
				--(*ref_count_);
				rhs.ref_count_ = nullptr;
				return *this;
			}

			if (ref_count_ != nullptr) {
				--(*ref_count_);
			}
			ref_count_ = rhs.ref_count_;
			_handle = std::move(rhs._handle);

			rhs.ref_count_ = nullptr;

			return *this;
		}

		constexpr ~RC() noexcept {
			if (ref_count_ == nullptr) {
				return;
			}

			--(*ref_count_);

			if (*ref_count_ == 0u) {
				Destroy(_handle);
				delete ref_count_;
			}
			ref_count_ = nullptr;
		}

		Res* operator->() noexcept {
			assert(ref_count_ != nullptr && "Cannot access invalid RC object.");
			return &_handle;
		}

		template<typename... Args> requires std::bool_constant<sizeof...(Args) != 0>::value
		constexpr void reset(Args&&... args) noexcept {
			::new (&_handle) Res{ std::forward<Args>(args)... };

			if (ref_count_ != nullptr) {
				if (*ref_count_ == 1u)
				{
					delete ref_count_;
				}
				else
				{
					--(*ref_count_);
				}
			}
			ref_count_ = new size_t(1u);
		}

		[[nodiscard]] 
		constexpr size_t get_ref_count() const noexcept {
			return ref_count_ != nullptr ? *ref_count_ : 0u;
		}

		[[nodiscard]]
		constexpr Res& get() noexcept {
			return _handle;
		}

		[[nodiscard]]
		constexpr const Res& get() const noexcept {
			return _handle;
		}

		[[nodiscard]] 
		constexpr bool is_empty() const noexcept {
			return ref_count_ == nullptr;
		}
	};
}