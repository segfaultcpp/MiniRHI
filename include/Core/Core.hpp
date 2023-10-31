#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <cstdio>

#ifdef ANDROID
namespace std
{
	template <class _From, class _To>
	concept convertible_to = __is_convertible_to(_From, _To) && requires {
		static_cast<_To>(std::declval<_From>());
	};

	template <class _Derived, class _Base>
	concept derived_from = __is_base_of(_Base, _Derived)
						   && __is_convertible_to(const volatile _Derived*, const volatile _Base*);

	template <class _FTy, class... _ArgTys>
	concept invocable = requires(_FTy&& _Fn, _ArgTys&&... _Args) {
		std::invoke(static_cast<_FTy&&>(_Fn), static_cast<_ArgTys&&>(_Args)...);
	};

	template <class _To, class _From,
			enable_if_t<conjunction_v<bool_constant<sizeof(_To) == sizeof(_From)>, is_trivially_copyable<_To>,
					is_trivially_copyable<_From>>,
					int> = 0>
	constexpr _To bit_cast(const _From& _Val) noexcept {
		return __builtin_bit_cast(_To, _Val);
	}
}
#endif

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

template<typename T>
concept PtrLike = std::is_pointer_v<T> || requires(T ptr) {
    ptr = T::kNullHandle;
};

/*
* Wrapper aroung pointer-like object that is guaranteed to be non-null.
* Non-null check occurs at object creation.
* Ptr type must satisfy PtrLike concept.
*/
template<typename Ptr> requires PtrLike<Ptr>
struct NonNull {
    static_assert(sizeof(Ptr) <= sizeof(void*));

    static constexpr auto kNullHandle = [] {
        if constexpr (std::is_pointer_v<Ptr>) {
            return nullptr;
        } else {
            return Ptr::kNullHandle;
        }
    }();

    Ptr ptr;

    explicit NonNull(std::nullptr_t) = delete;

    template<std::convertible_to<Ptr> U>
    [[nodiscard]]
    constexpr explicit NonNull(U pointer, auto&& fn) noexcept
        : ptr{ pointer }
    {
        if (ptr == kNullHandle) {
            fn();
            std::printf("Error! Trying to construct NonNull<T> object with nullptr!\n");
            std::abort();
        }
    }

    [[gnu::always_inline, nodiscard]]
    Ptr operator->() const noexcept {
        return ptr;
    }

    [[gnu::always_inline, nodiscard]]
    auto& operator*() const noexcept {
        return *ptr;
    }
};

template<typename T, typename U>
NonNull(T, U&&) -> NonNull<std::remove_reference_t<T>>;

template<typename T, typename Fn>
auto make_non_null(T ptr, Fn&& fn) noexcept {
    return NonNull<T>{ ptr, std::forward<Fn>(fn) };
}

template <std::size_t N>
class FixedString final {
public:
	constexpr FixedString(const char(&str)[N + 1]) noexcept {
		std::copy_n(str, N + 1, std::data(data));
	}

	constexpr FixedString(std::string_view sv) noexcept {
		std::copy_n(sv.data(), N, std::data(data));
	}

	[[nodiscard]] 
	constexpr auto operator<=>(const FixedString&) const = default;

	[[nodiscard]] 
	constexpr operator std::string_view() const {
		return { std::data(data), N };
	}

	[[nodiscard]] 
	constexpr std::size_t size() const { 
		return N; 
	}

	std::array<char, N + 1> data{};
};

template <std::size_t N>
FixedString(const char(&str)[N]) -> FixedString<N - 1>;

template<FixedString Str>
struct CTString {
	static constexpr FixedString kValue = Str;
};

template<std::size_t I, typename T>
struct Indexed {
    using Type = T;
};

namespace details {
	template<typename T, typename... Args>
	struct SameAsAny {
		static constexpr bool value = (std::is_same_v<T, Args> || ...);
	};
}

template<typename... Args>
inline constexpr bool same_as_any_v = details::SameAsAny<Args...>::value;

template<typename... Args>
concept SameAsAny = same_as_any_v<Args...>;
