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
		size_t* _refCount = nullptr;
		Res _handle;

	public:
		explicit RC() noexcept = default;

		template<typename... Args>
		explicit RC(Args&&... args) noexcept
			: _refCount(new size_t(1u))
			, _handle(std::forward<Args>(args)...)
		{}

		RC(const RC& rhs) noexcept
			: _refCount(rhs._refCount)
			, _handle(rhs._handle)
		{
			if (_refCount != nullptr)
			{
				++(*_refCount);
			}
		}

		RC& operator=(const RC& rhs) noexcept {
			if (_refCount != nullptr) {
				--(*_refCount);
			}

			// TODO: RE_ASSERT(rhs._refCount != nullptr, "Cannot copy null ref.");

			_refCount = rhs._refCount;
			++(*_refCount);

			_handle = rhs._handle;

			return *this;
		}

		RC(RC&& rhs) noexcept
			: _refCount(rhs._refCount)
			, _handle(std::move(rhs._handle))
		{
			rhs._refCount = nullptr;
		}

		RC& operator=(RC&& rhs) noexcept {
			if (_refCount == rhs._refCount) {
				rhs._refCount = nullptr;
				return *this;
			}

			--(*_refCount);
			_refCount = rhs._refCount;
			_handle = std::move(rhs._handle);

			rhs._refCount = nullptr;

			return *this;
		}

		~RC() noexcept {
			if (_refCount == nullptr) {
				return;
			}

			--(*_refCount);

			if (*_refCount == 0u) {
				Destroy(_handle);
				delete _refCount;
				_refCount = nullptr;
			}
		}

		Res* operator->() noexcept {
			return &_handle;
		}

		template<typename... Args>
		void reset(Args&&... args) noexcept {
			::new (&_handle) Res{ std::forward<Args>(args)... };

			if (_refCount != nullptr) {
				if (*_refCount == 1u)
				{
					delete _refCount;
				}
				else
				{
					--(*_refCount);
				}
			}
			_refCount = new size_t(1u);
		}

		[[nodiscard]] 
		size_t get_ref_count() const noexcept {
			return _refCount != nullptr ? *_refCount : 0u;
		}

		[[nodiscard]]
		Res& get() noexcept {
			return _handle;
		}

		[[nodiscard]]
		const Res& get() const noexcept {
			return _handle;
		}

		[[nodiscard]] 
		bool is_empty() const noexcept {
			return _refCount == nullptr;
		}

	};
}