#pragma once

#include "internal.hpp"

namespace winnow::token
{

inline constexpr struct
{
    template <typename CharT>
    static constexpr auto operator()(const CharT* str)
    {
        return operator()(std::basic_string_view<CharT>(str));
    }

    template <typename CharT>
    static constexpr auto operator()(std::basic_string_view<CharT> str)
    {
        return [=]<typename Stream>(Stream& stream) 
        {
            return detail::literal_parser<Stream>(str)(stream);
        };
    }
} literal;

    

}  // namespace winnow
