/*
    Different from rust::nom, our character parsers always return string_view.
    For some parsers which return char(AsChar), we return a string_view of length 1 instead.
*/

#pragma once

#include <leviathan/extc++/chars.hpp>
#include "internal.hpp"

namespace nom::character
{

inline constexpr auto multispace0 = ConditionalLoop0(MultiSpace());
inline constexpr auto multispace1 = ConditionalLoop1(MultiSpace(), ErrorKind::MultiSpace);

inline constexpr auto digit0 = ConditionalLoop0(&::isdigit);
inline constexpr auto digit1 = ConditionalLoop1(&::isdigit, ErrorKind::Digit);

inline constexpr auto alphanumeric0 = ConditionalLoop0(&::isalnum);
inline constexpr auto alphanumeric1 = ConditionalLoop1(&::isalnum, ErrorKind::AlphaNumeric);

inline constexpr auto alpha0 = ConditionalLoop0(&::isalpha);
inline constexpr auto alpha1 = ConditionalLoop1(&::isalpha, ErrorKind::Alpha);

inline constexpr auto space0 = ConditionalLoop0(Space());
inline constexpr auto space1 = ConditionalLoop1(Space(), ErrorKind::Space);

/////////////////////////////////////////////////////////////////////////////

inline constexpr auto one_of = [](std::string_view s) static
{
    return CheckFirstCharacter(std::bind_front([=](char c) { return s.contains(c); }), ErrorKind::OneOf);
};

inline constexpr auto none_of = [](std::string_view s) static
{
    return CheckFirstCharacter(std::bind_front([=](char c) { return !s.contains(c); }), ErrorKind::NoneOf);
};

inline constexpr auto satisfy = []<typename Pred>(Pred&& pred) static
{
    return CheckFirstCharacter((Pred&&)pred, ErrorKind::Satisfy);
};

inline constexpr auto char_ = [](char ch) static
{
    return CheckFirstCharacter(std::bind_front(std::equal_to<char>(), ch), ErrorKind::Char);
};

inline constexpr auto tab = char_('\t');
inline constexpr auto newline = char_('\n');

inline constexpr auto anychar = CheckFirstCharacter([](auto ch) static { return true; }, ErrorKind::Eof);

inline constexpr auto bin_digit0 = ConditionalLoop0(cpp::isbindigit);
inline constexpr auto bin_digit1 = ConditionalLoop1(cpp::isbindigit, ErrorKind::BinDigit);

inline constexpr auto oct_digit0 = ConditionalLoop0(cpp::isoctdighit);
inline constexpr auto oct_digit1 = ConditionalLoop1(cpp::isoctdighit, ErrorKind::OctDigit);

inline constexpr auto hex_digit0 = ConditionalLoop0(cpp::ishexdigit);
inline constexpr auto hex_digit1 = ConditionalLoop1(cpp::ishexdigit, ErrorKind::HexDigit);

inline constexpr auto crlf = Ctrl();
inline constexpr auto line_ending = LineEnding();
inline constexpr auto not_line_ending = NotLineEnding();

}  // namespace nom

