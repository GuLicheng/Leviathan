#ifndef MYPRINT_H
#define MYPRINT_H


#include <ranges>
#include <iostream>
namespace std {

    //tuple-like type for operator<<
    template<class Ch, class Tr, class Tuple>
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
    struct print_view : public rg::view_interface<print_view<V>>, private V {
        using V::V;
        using V::begin;
        using V::end;
    };
    template<typename _Range>
    print_view(_Range&&) -> print_view<views::all_t<_Range>>;

namespace views{
    inline constexpr __adaptor::_RangeAdaptorClosure print
    = []<viewable_range R>(R&& r) { return print_view{ ::std::forward<R>(r) }; };
}// namespace views
}// namespace ranges

    template<class Ch, class Tr, class V>
    auto& operator << ( std::basic_ostream<Ch, Tr>& os, rg::print_view<V>&& t ) {
        int flag = -1;
        for (auto&& elem : t) {
            os << ( ++flag ? ", " : "(" ) << elem;
        }
        return os << ")";
    }
} // namespace std


#endif //MYPRINT_H
