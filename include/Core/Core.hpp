#pragma once
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <concepts>
#include <type_traits>
#include <cassert>
#include <cstdio>

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
using f80 = long double;

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