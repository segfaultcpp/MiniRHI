#pragma once
#include <concepts>
#include <cassert>

#include <Core/Core.hpp>
#include <type_traits>

namespace minirhi
{
	template<typename Res, typename Res::DestroyFn DestroyFn>
		requires std::is_trivially_copyable_v<Res>
	class RC
	{
	protected:
		size_t* _refCount = nullptr;
		Res _handle;

	public:
		RC() noexcept
			: _refCount(nullptr)
		{}

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

		RC& operator=(const RC& rhs) noexcept
		{
			if (_refCount != nullptr)
			{
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

		RC& operator=(RC&& rhs) noexcept
		{
			if (_refCount == rhs._refCount)
			{
				rhs._refCount = nullptr;
				return *this;
			}

			--(*_refCount);
			_refCount = rhs._refCount;
			_handle = std::move(rhs._handle);

			rhs._refCount = nullptr;

			return *this;
		}

		~RC()
		{
			if (_refCount == nullptr)
				return;

			--(*_refCount);

			if (*_refCount == 0u)
			{
				DestroyFn(_handle);
				delete _refCount;
				_refCount = nullptr;
			}
		}

	public:
		Res* operator->() noexcept
		{
			return &_handle;
		}

	public:
		template<typename... Args>
		void Reset(Args&&... args) noexcept
		{
			::new (&_handle) Res{ std::forward<Args>(args)... };

			if (_refCount != nullptr)
			{
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

	public:
		size_t GetRefCount() const noexcept
		{
			return _refCount ? *_refCount : 0u;
		}

		Res& Get() noexcept
		{
			return _handle;
		}

		const Res& Get() const noexcept
		{
			return _handle;
		}

		bool IsEmpty() const noexcept
		{
			return _refCount == nullptr;
		}

	};
}