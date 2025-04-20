#pragma once

#include <leviathan/extc++/concepts.hpp>
#include <leviathan/extc++/hash.hpp>

namespace cpp
{
    
struct tuple_hasher
{
    template <meta::tuple_like Tuple>
    static constexpr size_t operator()(const Tuple& t)
    {
        return [&]<size_t... Idx>(std::index_sequence<Idx...>)
        {
            return hash::hash_combine(std::get<Idx>(t)...);
        }(std::make_index_sequence<std::tuple_size_v<Tuple>>());
    } 
};


} // namespace cpp

