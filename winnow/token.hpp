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
            static_assert(std::is_same_v<CharT, typename Stream::value_type>, "Stream value type must match literal type");
            return detail::literal_parser<Stream>(str)(stream);
        };
    }
} literal;

inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min, std::optional<size_t> max = std::nullopt)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            return detail::take_while_parser<Stream, Pred>(pred, min, max)(stream);
        };
    }
} take_while;
    
inline constexpr struct
{
    template <typename Pred>
    static constexpr auto operator()(Pred pred, size_t min, std::optional<size_t> max = std::nullopt)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            auto fn = std::not_fn(std::move(pred));
            return detail::take_while_parser<Stream, decltype(fn)>(std::move(fn), min, max)(stream);
        };
    }
} take_till;

inline constexpr struct
{
    static constexpr auto operator()(size_t count)
    {
        return [=]<typename Stream>(Stream& stream)
        {
            return detail::take_parser<Stream>(count)(stream);
        };
    }
} take;

}  // namespace winnow::token
