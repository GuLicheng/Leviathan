#ifndef MYPRINT_H
#define MYPRINT_H

#include "myranges.h"
#include <iostream>
#include <charconv>
#include <cstddef>
#include <span>
namespace std {

    //tuple-like type for operator<<
    template< class Ch, class Tr, class Tuple >
    requires requires { typename tuple_size<remove_reference_t<Tuple>>::type; }
    auto& operator << ( std::basic_ostream<Ch, Tr>& os, Tuple&& t ) {
        os << "(";
        [&] <std::size_t... Is> ( index_sequence<Is...> )
        { ( ( os << ( Is == 0 ? "" : ", " ) << std::get<Is>( forward<Tuple>(t) ) ), ...); }
                ( make_index_sequence<tuple_size_v<remove_reference_t<Tuple>>> {} );
        return os << ")";
    }

namespace rg = ::std::ranges;

namespace ranges {
    template< rg::view V >
    struct print_view;

    namespace views {
        inline constexpr __adaptor::_RangeAdaptorClosure print
                = []<viewable_range R>(R &&r) { return print_view(::std::forward<R>(r)); };
    }
    template< rg::view V >
    struct print_view : public rg::view_interface<print_view<V>>, V {
        constexpr print_view(V v) : V( ::std::move(v) ) { }
        using V::begin;
        using V::end;
        template< class Ch, class Tr >
        friend auto& operator << ( std::basic_ostream<Ch, Tr>& os, print_view& t ) {
            int flag = -1;
            for (auto&& elem : t) {
                os << ( ++flag ? ", " : "(" );
                if constexpr (requires { os << elem; })
                    os << elem;
                else
                    os << views::print(elem);
            }
            return os << ")";
        }
        template< class Ch, class Tr >
        friend auto& operator << ( std::basic_ostream<Ch, Tr>& os, print_view&& t ) {
            int flag = -1;
            for (auto&& elem : t) {
                os << ( ++flag ? ", " : "(" );
                if constexpr (requires { os << elem; })
                    os << elem;
                else
                    os << views::print(elem);
            }
            return os << ")";
        }
    };
    template< class R >
    print_view(R&&) -> print_view< views::all_t<R> >;

    template<rg::input_range V> requires rg::view<V> && (sizeof (rg::range_value_t<V>) == 1)
    struct print_bit_view : public rg::view_interface<print_view<V>>, V {
    constexpr print_bit_view(V v) : V( ::std::move(v) ) { }
        using V::begin;
        using V::end;
        template< class Ch, class Tr >
        friend auto& operator << ( std::basic_ostream<Ch, Tr>& os, const print_bit_view& t ) {
            for (auto [i, ch]: t | views::enumerate) {
                os.width(2);
                os.fill('0');
                os << std::hex << int(ch);
                if (i & 1) os << ' ';
            }
            return os;
        }
    };
    template< class R >
    print_bit_view(R&&) -> print_bit_view< views::all_t<R> >;
    namespace views {
        inline constexpr auto print_bit = [](auto&& any) {
                    auto sz = sizeof(any);
                    auto first = reinterpret_cast<::std::uint8_t*>(::std::addressof(any));
                    ::std::span<::std::uint8_t> s( first, sz );
                    return print_bit_view(::std::move(s));
                };
    }

} // namespace ranges
} // namespace std


#endif //MYPRINT_H
