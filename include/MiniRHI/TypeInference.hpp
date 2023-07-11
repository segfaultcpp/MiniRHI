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
 *  Supports up to 7 elements in a struct.
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

        template<typename T, typename... Ty>
        concept BracesConstructibleFrom = requires(T t)
        {
            std::is_pod_v<T>;
            { T{ std::declval<Ty>()... } } -> std::same_as<T>;
        };

        template<typename T, size_t N, typename... Ty>
        constexpr auto IsBracesConstructibleN()
        {
            if constexpr (N == 0)
            {
                return BracesConstructibleFrom<T, Ty...>;
            }
            else
            {
                return IsBracesConstructibleN<T, N - 1, AnyType, Ty...>();
            }
        }

        template<typename T1>
        struct Deduce;

#define _MINIRHI_SPECIALIZE_FOR_STD_ARRAY(Type, Size, Format_) \
	template<> \
	struct Deduce<std::array<Type, Size>> \
	{ \
		using Fmt = ::minirhi::format::Format_; \
	};

#define _MINIRHI_SPECIALIZE_T(Type, Format_) \
	template<> \
	struct Deduce<Type> \
	{ \
		using Fmt = ::minirhi::format::Format_; \
	};

        _MINIRHI_SPECIALIZE_T(u8, R8UInt_t);
        _MINIRHI_SPECIALIZE_T(u16, R16UInt_t);
        _MINIRHI_SPECIALIZE_T(u32, R32UInt_t);
        
        _MINIRHI_SPECIALIZE_T(f32, R32Float_t);

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
        consteval auto convert() noexcept /* -> ToVtxArr<type0, type1, ...> */
        {
            using type = std::decay_t<T>;
            T object = *std::bit_cast<T*>(nullptr); // safe: code never runs

            if constexpr (IsBracesConstructibleN<type, 6>())
            {
                auto&& [p1, p2, p3, p4, p5, p6] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5), decltype(p6)>{};
            }
        	else if constexpr (IsBracesConstructibleN<type, 5>())
            {
                auto&& [p1, p2, p3, p4, p5] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3), decltype(p4), decltype(p5)>{};
            }
            else if constexpr (IsBracesConstructibleN<type, 4>())
            {
                auto&& [p1, p2, p3, p4] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3), decltype(p4)>{};
            }
            else if constexpr (IsBracesConstructibleN<type, 3>())
            {
                auto&& [p1, p2, p3] = object;
                return ToVtxArr<decltype(p1), decltype(p2), decltype(p3)>{};
            }
            else if constexpr (IsBracesConstructibleN<type, 2>())
            {
                auto&& [p1, p2] = object;
                return ToVtxArr<decltype(p1), decltype(p2)>{};
            }
            else if constexpr (IsBracesConstructibleN<type, 1>())
            {
                auto&& [p1] = object;
                return ToVtxArr<decltype(p1)>{};
            }
            else {
                return ToVtxArr<> {};
            }
        }

        template<typename T>
        using Convert = typename decltype(convert<T>())::Ty;

        template<typename T>
        struct IsVtxAttrArrEmpty;

        template<typename... Ty>
        struct IsVtxAttrArrEmpty<minirhi::VtxAttrArr<Ty...>>
        {
            static constexpr bool Value = 0 == sizeof...(Ty);
        };

        template<typename T>
        concept HasGetAttrs = !IsVtxAttrArrEmpty<decltype(T::get_attrs())>::Value;

        template<typename T>
        concept CanInferAttrs = !std::is_same_v<minirhi::VtxAttrArr<>, Convert<T>>;
    }

    template<typename T>
    concept TIsVertex = std::is_pod_v<T> && (detail_ti::HasGetAttrs<T> || detail_ti::CanInferAttrs<T>);

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

            //using At = MakeVertexAttributes<Vertex2>;
        };

        struct NonVertex
        {};

        static_assert(detail_ti::HasGetAttrs<Vertex2>);
        static_assert(std::is_same_v<MakeVertexAttributes<Vertex1>, decltype(Vertex2::get_attrs())>);
        static_assert(TIsVertex<Vertex1>);
        static_assert(TIsVertex<Vertex2>);
        static_assert(!TIsVertex<NonVertex>);
    }
}
