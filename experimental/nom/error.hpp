#pragma once

#include "rust.hpp"

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
  
using rust::Unit;
using rust::Result;

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

template <typename ErrorCode>
struct Error
{ 
    std::string info;
    ErrorCode code;
};

template <typename T, typename E = ErrorKind>
using IResult = Result<T, Error<E>>;

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

// template <typename T1, typename E1, typename T2, typename E2>
// struct cpp::type_caster<nom::IResult<T1, nom::Error<E1>>, nom::IResult<T2, nom::Error<E2>>>
// {
//     using result_type = nom::IResult<T1, E1>;

//     static constexpr result_type operator()(const nom::IResult<T2, E2>& result2)
//     {
//         if (result2)
//         {
//             return result_type(std::in_place, static_cast<T1>(*value));
//         }
//         else
//         {
//             return result_type(std::unexpect, Error<E1>{ .info = value.error().info, .code = static_cast<E1>(value.error().code) });
//         }
//     }
// };