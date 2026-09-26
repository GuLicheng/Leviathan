#pragma once

#include <leviathan/config_parser/core/internal.hpp>
#include <leviathan/config_parser/core/parser_interface.hpp>
#include <type_traits>


namespace cpp::config::parser
{

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred&& pred, occurrences<size_t> range) 
    {
        return detail::take_while_parser<std::decay_t<Pred>>((Pred&&) pred, range);
    }
} take_while;

inline constexpr auto alpha0 = take_while(::isalpha, { 0, std::nullopt });
inline constexpr auto alpha1 = take_while(::isalpha, { 1, std::nullopt });

inline constexpr auto digit0 = take_while(::isdigit, { 0, std::nullopt });
inline constexpr auto digit1 = take_while(::isdigit, { 1, std::nullopt });

inline constexpr auto alnum0 = take_while(::isalnum, { 0, std::nullopt });
inline constexpr auto alnum1 = take_while(::isalnum, { 1, std::nullopt });

inline constexpr auto space0 = take_while(::isspace, { 0, std::nullopt });
inline constexpr auto space1 = take_while(::isspace, { 1, std::nullopt });

inline constexpr auto hexdigit0 = take_while(::isxdigit, { 0, std::nullopt });
inline constexpr auto hexdigit1 = take_while(::isxdigit, { 1, std::nullopt });

}  // namespace cpp::config::parser
