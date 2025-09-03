/*
    Combinators to be implemented:

    x - complete
    x - consumed
    x - cut
    x - flat_map
    x - into
    x - iterator
    x - map_opt
*/

#pragma once

#include <leviathan/extc++/concepts.hpp>
#include "error.hpp"

namespace nom::combinator
{

// inline constexpr struct
// {
//     template <typename F>
//     static constexpr auto operator()(F&& f) 
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::decay_t<ParserFunction>, ParseContext&>;
//             using R = std::optional<typename R1::value_type>;

//             auto [parser] = (FunctionTuple&&)fns;
//             auto result = parser(ctx);

//             return result ? R(std::in_place, std::make_optional(std::move(*result))) 
//                           : R(std::nullopt);
//         };
//         return make_parser_binder(fn, (F&&)f);
//     }
// } map_opt;

// inline constexpr struct
// {
//     template <typename F1>
//     static constexpr auto operator()(bool b, F1&& f1)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             auto [cond, parser] = (FunctionTuple&&)fns;
//             using R1 = decltype(parser(ctx));
//             using R = IResult<std::optional<typename R1::value_type>>;

//             if (cond)
//             {
//                 auto result = parser(ctx);

//                 return result ? R(std::in_place, std::in_place, std::move(*result))
//                               : R(std::unexpect, std::move(result.error()));
//             }
//             else
//             {
//                 return R(std::in_place, std::nullopt);
//             }
//         };
//         return make_parser_binder(fn, b, (F1&&)f1);
//     }
// } cond;

// inline constexpr struct
// {
//     template <typename F1>
//     static constexpr auto operator()(F1&& f1)
//     {   
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
//             using R = IResult<typename R1::value_type>;

//             auto [parser] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy
//             auto result = parser(clone);

//             if (result)
//             {
//                 ctx = std::move(clone);
//                 return ctx.empty() 
//                      ? R(std::in_place, std::move(*result))
//                      : R(std::unexpect, std::string(ctx.begin(), ctx.end()), ErrorKind::Eof);
//             }
//             else
//             {
//                 return R(std::unexpect, std::move(result.error()));
//             }
//         };
//         return make_parser_binder(fn, (F1&&)f1);
//     }
// } all_consuming;

// inline constexpr struct
// {
//     template <typename F>
//     static constexpr auto operator()(F&& f) 
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R = IResult<Unit>;

//             auto [parser] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy
//             auto result = parser(clone);

//             if (result)
//             {
//                 return R(std::unexpect, std::string(ctx.begin(), ctx.end()), ErrorKind::Not);
//             }
//             else
//             {
//                 return R(std::in_place, Unit());
//             }
//         };
//         return make_parser_binder(fn, (F&&)f);
//     }
// } not_;

// inline constexpr struct
// {
//     template <typename ParserFunction, typename MapFunction>
//     static constexpr auto operator()(ParserFunction&& pf, MapFunction&& mf)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::decay_t<ParserFunction>, ParseContext&>;
//             using R2 = std::invoke_result_t<std::decay_t<MapFunction>, typename R1::value_type>;

//             constexpr bool optional_or_expected = cpp::meta::specialization_of<R2, std::optional> 
//                                                || cpp::meta::specialization_of<R2, std::expected>;
            
//             auto [parser, mapper] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy
//             auto result = parser(ctx);

//             if constexpr (optional_or_expected)
//             {
//                 using ValueType = typename R2::value_type;
//                 using R = IResult<ValueType>;
                
//                 if (!result)
//                 {
//                     return R(std::unexpect, std::move(result.error()));
//                 }

//                 auto mapped_result = mapper(std::move(*result));

//                 if (!mapped_result)
//                 {
//                     ctx = std::move(clone);
//                     return R(std::unexpect, std::string(ctx.begin(), ctx.end()), ErrorKind::MapRes);
//                 }
//                 else
//                 {
//                     return R(std::in_place, std::move(*mapped_result));
//                 }
//             }
//             else
//             {
//                 // We provide an exception version here since most of functions
//                 // in C++ use exception instead of error code.
//                 using R = IResult<R2>;

//                 if (!result)
//                 {
//                     return R(std::unexpect, std::move(result.error()));
//                 }
                
//                 try
//                 {
//                     auto mapped_result = mapper(std::move(*result));
//                     return R(std::in_place, std::move(mapped_result));
//                 }
//                 catch(...)
//                 {
//                     ctx = std::move(clone);
//                     return R(std::unexpect, std::string(ctx.begin(), ctx.end()), ErrorKind::MapRes);
//                 }
//             }
//         };

//         return make_parser_binder(fn, (ParserFunction&&)pf, (MapFunction&&)mf); 
//     }   
// } map_res;
    
// inline constexpr struct 
// {
//     template <typename T, typename F>
//     static constexpr auto operator()(T&& v, F&& f)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R = IResult<std::decay_t<T>>;

//             auto [value, parser] = (FunctionTuple&&)fns;
//             auto result = parser(ctx);

//             return result ? R(std::in_place, std::move(value)) 
//                           : R(std::unexpect, std::move(result.error()));
//         };
//         return make_parser_binder(fn, (T&&)v, (F&&)f);
//     }
// } value;

// inline constexpr struct 
// {
//     template <typename F1>
//     static constexpr auto operator()(F1&& f1)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             auto [f] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy
//             auto result = f(clone);
//             return result;
//         };
//         return make_parser_binder(fn, (F1&&)f1);
//     }
// } peek;

// inline constexpr struct
// {
//     template <typename ParserFunction, typename MapFunction>
//     static constexpr auto operator()(ParserFunction&& pf, MapFunction&& mf)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::decay_t<ParserFunction>, ParseContext&>;
//             using R = IResult<std::invoke_result_t<std::decay_t<MapFunction>, typename R1::value_type>>;

//             auto [parser, mapper] = (FunctionTuple&&)fns;
//             auto result = parser(ctx);

//             return result ? R(std::in_place, mapper(std::move(*result))) 
//                           : R(std::unexpect, std::move(result.error()));
//         };

//         return make_parser_binder(fn, (ParserFunction&&)pf, (MapFunction&&)mf); 
//     }
// } map;

// inline constexpr struct
// {
//     template <typename F1, typename F2>
//     static constexpr auto operator()(F1&& f1, F2&& f2)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R2 = std::invoke_result_t<std::decay_t<F2>, ParseContext&>;
//             using R = IResult<typename R2::value_type>;

//             auto [first, second] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy
//             auto result1 = first(ctx);

//             if (!result1)
//             {
//                 return R(std::unexpect, std::move(result1.error()));
//             }

//             auto result2 = second(*result1);

//             if (!result2)
//             {
//                 ctx = std::move(clone);
//                 return R(std::unexpect, std::move(result2.error()));
//             }

//             return R(std::in_place, std::move(*result2));
//         };
//         return make_parser_binder(fn, (F1&&)f1, (F2&&)f2);
//     }
// } map_parser;

// inline constexpr struct
// {
//     template <typename F1>
//     static constexpr auto operator()(F1&& f1)
//     {   
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
//             using Optional = std::optional<typename R1::value_type>;
//             using R = IResult<Optional>;

//             auto [parser] = (FunctionTuple&&)fns;
//             auto result = parser(ctx);
            
//             return result ? R(std::in_place, std::in_place, std::move(*result))
//                           : R(std::in_place, std::nullopt); // always succeed
//         };
//         return make_parser_binder(fn, (F1&&)f1);
//     }
// } opt;

// inline constexpr struct
// {
//     template <typename ParseContext>
//     static constexpr auto operator()(ParseContext& ctx)
//     {
//         using R = IResult<std::string_view>;    
//         auto result = std::string_view(ctx.begin(), ctx.end());

//         return ctx.empty() 
//              ? R(std::in_place, result)
//              : R(std::unexpect, std::string(result), ErrorKind::Eof);
//     }   
// } eof;

// inline constexpr struct
// {
//     template <typename ParseContext>
//     static constexpr auto operator()(ParseContext& ctx)
//     {
//         return IResult<std::string_view>(
//             std::unexpect, 
//             std::string(ctx.begin(), ctx.end()), 
//             ErrorKind::Fail
//         );
//     }       
// } fail;

// inline constexpr struct
// {
//     template <typename F1>
//     static constexpr auto operator()(F1&& f1)
//     {   
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R = IResult<std::string_view>;

//             auto [parser] = (FunctionTuple&&)fns;
//             auto clone = ctx; // copy

//             if (auto result = parser(ctx); result)
//             {
//                 std::string_view matched(clone.begin(), ctx.begin());
//                 return R(std::in_place, matched);
//             }
//             else
//             {
//                 return R(std::unexpect, std::move(result.error()));
//             }
//         };
//         return make_parser_binder(fn, (F1&&)f1);
//     }
// } recognize;

// inline constexpr struct
// {
//     template <typename ParseContext>
//     static constexpr auto operator()(ParseContext& ctx)
//     {
//         auto clone = ctx;
//         ctx.remove_prefix(ctx.size());
//         return IResult<std::string_view>(
//             std::in_place, 
//             std::string_view(clone.begin(), clone.end())
//         );
//     }
// } rest;

// inline constexpr struct
// {
//     template <typename ParseContext>
//     static constexpr auto operator()(ParseContext& ctx)
//     {
//         return IResult<size_t>(std::in_place, ctx.size());
//     }
// } rest_len;

// inline constexpr struct
// {
//     template <typename T>
//     static constexpr auto operator()(T&& v)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R = IResult<std::decay_t<T>>;

//             auto [value] = (FunctionTuple&&)fns;
//             return R(std::in_place, std::move(value));
//         };
//         return make_parser_binder(fn, (T&&)v);
//     }
// } success;

// inline constexpr struct
// {   
//     template <typename F1, typename F2>
//     static constexpr auto operator()(F1&& f1, F2&& f2)
//     {
//         auto fn = []<typename FunctionTuple, typename ParseContext>(FunctionTuple&& fns, ParseContext& ctx) static
//         {
//             using R1 = std::invoke_result_t<std::tuple_element_t<0, std::decay_t<FunctionTuple>>, ParseContext&>;
//             using R = IResult<typename R1::value_type>;

//             auto [parser, verifier] = (FunctionTuple&&)fns;
//             auto clone = ctx;
//             auto result = parser(ctx);

//             if (!result)
//             {
//                 return R(std::unexpect, std::move(result.error()));
//             }

//             if (!verifier(*result))
//             {
//                 ctx = std::move(clone);
//                 return R(std::unexpect, std::string(ctx.begin(), ctx.end()), ErrorKind::Verify);
//             }

//             return R(std::in_place, std::move(*result));
//         };
//         return make_parser_binder(fn, (F1&&)f1, (F2&&)f2); 
//     }
// } verify;



} // namespace nom

