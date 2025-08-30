#pragma once

#include <utility>
#include <stdexcept>
#include <expected>
#include <string_view>
#include <string>
#include <variant>
#include <format>
#include <expected>
#include <optional>

namespace nom
{
  
enum class ErrorKind
{
    Ok,

    Tag,
    MapRes,
    MapOpt,
    Alt,
    IsNot,
    IsA,
    SeparatedList,
    SeparatedNonEmptyList,
    Many0,
    Many1,
    ManyTill,
    Count,
    TakeUntil,
    LengthValue,
    TagClosure,
    Alpha,
    Digit,
    HexDigit,
    OctDigit,
    BinDigit,
    AlphaNumeric,
    Space,
    MultiSpace,
    LengthValueFn,
    Eof,
    Switch,
    TagBits,
    OneOf,
    NoneOf,
    Char,
    CrLf,
    RegexpMatch,
    RegexpMatches,
    RegexpFind,
    RegexpCapture,
    RegexpCaptures,
    TakeWhile1,
    Complete,
    Fix,
    Escaped,
    EscapedTransform,
    NonEmpty,
    ManyMN,
    Not,
    Permutation,
    Verify,
    TakeTill1,
    TakeWhileMN,
    TooLarge,
    Many0Count,
    Many1Count,
    Float,
    Satisfy,
    Fail,
    Many,
    Fold,
    Precedence,
    
    Custom,
    Sentinel,
};

// I am not sure how to map `()` in rust to C++.
// Maybe use std::tuple<> or define an empty struct?
// https://doc.rust-lang.org/nightly/std/primitive.unit.html
// It seems like `void` in C++ but a complete type.
using Unit = std::tuple<>;

template <typename Information>
struct Error
{ 
    Information info;
    ErrorKind code;
};

struct Incomplete 
{
    size_t needed; // number of additional bytes needed
};

template <typename Failure, typename Error = Failure>
class Err
{
    std::variant<Incomplete, Failure, Error> m_value;

public:


};

template <typename T, typename E = Error<std::string>>
using IResult = std::expected<T, E>;

struct ParseError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

template <typename F>
struct Context
{
    std::string context;
    F parser;

    template <typename ParseContext>
    constexpr auto operator()(ParseContext& ctx)
    {
        auto result = parser(ctx);

        if (!result)
        {
            result.error().info = std::format("{}: {}", context, result.error().info);
        }

        return result;
    }
};

inline constexpr struct
{
    template <typename F>
    static constexpr Context<F> operator()(std::string ctx, F&& parser)
    {
        return { .context = std::move(ctx), .parser = (F&&)parser };
    }
} context;

} // namespace nom

