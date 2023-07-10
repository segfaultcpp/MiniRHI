#pragma once
#include <concepts>
#include "Format.hpp"
#include "PipelineState.hpp"

/*
 *  VERTEX TYPE ATTRIBUTES DEDUCTION
 *
 *  If you have a `T::get_attribs()` function on your vertex class, it
 *  will use that. Otherwise, it will deduce what fields are in your Vertex struct
 *  and return you the `minirhi::minirhi::VtxAttrArr<...>`.
 *  
 *  Example:
 *      struct Vertex
 *      {
 *          std::array<f32, 2> position;
 *          std::array<f32, 3> color;
 *      }
 *      using Attribs = minirhi::MakeVertexAttributes<Vertex>;
 *
 */

namespace minirhi
{
    namespace detail_ti
    {
        struct AnyType
        {
            template<class T>
            constexpr operator T();
        };

        template<typename...>
        struct Repeat {};

        template<size_t T, typename T1, typename... Ty>
        auto repeat()
        {
            if constexpr (T == 0)
            {
                return Repeat<Ty...>{};
            }
            else
            {
                return repeat<T - 1, T1, T1, Ty...>();
            }
        }

        template <class T, class... TArgs>
        decltype(void(T{ std::declval<TArgs>()... }), std::true_type{}) test_is_braces_constructible(int);

        template <class, class...>
        std::false_type test_is_braces_constructible(...);

        template<typename T1, typename T2>
        struct IsConstructibleN;

        template<typename T1, typename... Ty>
        struct IsConstructibleN<T1, Repeat<Ty...>>
        {
            using Value = decltype(test_is_braces_constructible<T1, Ty...>(0));
        };

        template<typename T, size_t N>
        using IsBracesConstructible = IsConstructibleN<T, decltype(repeat<N, AnyType>())>::Value;

        template<typename T1>
        struct Deduce;

#define _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(Type, Size, Format_) \
	template<> \
	struct Deduce<std::array<Type, Size>> \
	{ \
		using Fmt = ::minirhi::format::Format_; \
	};

        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(f32, 1, R32Float_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(f32, 2, RG32Float_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(f32, 3, RGB32Float_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(f32, 4, RGBA32Float_t);

    	_MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u8, 1, R8UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u8, 2, RG8UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u8, 3, RGB8UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u8, 4, RGBA8UInt_t);

        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u16, 1, R16UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u16, 2, RG16UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u16, 3, RGB16UInt_t);
        _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(u16, 4, RGBA16UInt_t);

        // TODO: others ^^^


        template<typename... T>
        struct ToVtxArr
        {
            using Ty = minirhi::VtxAttrArr<minirhi::VtxAttr<typename Deduce<T>::Fmt>...>;
        };

        template<class T>
        constexpr auto convert(T&& object) noexcept /* -> Temp<type0, type1, ...> */
        {
            using type = std::decay_t<T>;
            if constexpr (IsBracesConstructible<type, 5>::value)
            {
                auto&& [p1, p2, p3, p4, p5] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5)>{};
            }
            else if constexpr (IsBracesConstructible<type, 4>::value)
            {
                auto&& [p1, p2, p3, p4] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3), decltype(p4)>{};
            }
            else if constexpr (IsBracesConstructible<type, 3>::value)
            {
                auto&& [p1, p2, p3] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3)>{};
            }
            else if constexpr (IsBracesConstructible<type, 2>::value)
            {
                auto&& [p1, p2] = object;
                return ToVtxArr<decltype(p1), decltype(p2)>{};
            }
            else if constexpr (IsBracesConstructible<type, 1>::value)
            {
                auto&& [p1] = object;
                return ToVtxArr<decltype(p1)>{};
            }
            else {
                throw "Struct does not have any attrs!";
            }
        }

        template<typename T>
        using Convert = typename decltype(convert(T{}))::Ty;

        template<typename T>
        concept HasGetAttrs = requires(T t)
        {
            // TODO: type
            T::get_attrs();
        };

        template<typename T>
        concept CanInferAttrs = requires(T t)
        {
            !std::is_same_v<minirhi::VtxAttrArr<>, Convert<T>>;
        };
    }

    template<typename T>
    concept TIsVertex = detail_ti::HasGetAttrs<T> || detail_ti::CanInferAttrs<T>;

    template<typename T>
    constexpr auto MakeVertexAttributes_t()
    {
	    if constexpr (detail_ti::HasGetAttrs<T>)
	    {
            return T::get_attrs();
	    }

        return detail_ti::Convert<T> {};
    }

    template<typename T>
    using MakeVertexAttributes = decltype(MakeVertexAttributes_t<T>());

    namespace test_type_deduction
    {
	    struct Vertex1
	    {
            std::array<f32, 2> f32_2;
            std::array<f32, 3> f32_3;
            std::array<u8, 2> u8_2;
        };

        struct Vertex2
    	{
            std::array<f32, 2> f32_2;
            std::array<f32, 3> f32_3;
        	std::array<u8, 2> u8_2;

            static constexpr auto get_attrs() noexcept
        	{
                return minirhi::VtxAttrArr<
                    minirhi::VtxAttr<minirhi::format::RG32Float_t>,
                    minirhi::VtxAttr<minirhi::format::RGB32Float_t>,
					minirhi::VtxAttr<minirhi::format::RG8UInt_t>
                >{};
            }
        };

        static_assert(std::is_same_v<MakeVertexAttributes<Vertex1>, decltype(Vertex2::get_attrs())>);
        static_assert(TIsVertex<Vertex1>);
        static_assert(TIsVertex<Vertex2>);
    }
}
