#ifndef __MEMORY_INVOKE_HPP__
#define __MEMORY_INVOKE_HPP__


#if defined(__GNUC__)
#define GNU_EXTEND_AVALIABLE 
#endif

#include <tuple>  // for std::tuple
#include <functional>  // for std::hash

// for algorithm
#include <algorithm>

// for container, the last tow are gnu extend
#include <list>
#include <unordered_map>
#include <map>

#ifdef GNU_EXTEND_AVALIABLE

#include <ext/pb_ds/tree_policy.hpp>
#include <ext/pb_ds/assoc_container.hpp>

#endif

// for function_traits
#include <leviathan/meta/type_list.hpp>

// extend std::hash
namespace std
{
template <typename... Ts>
struct hash<std::tuple<Ts...>>
{
    constexpr auto operator()(const std::tuple<Ts...>& t) const noexcept
    {
        return this->hash_impl(t, std::make_index_sequence<sizeof...(Ts)>());
    }

private:
    template <size_t... Idx>
    constexpr auto
    hash_impl(const std::tuple<Ts...>& t, std::index_sequence<Idx...>) const noexcept
    {
        return ((std::hash<Ts>()(std::get<Idx>(t))) + ...);
        // return (std::get<Idx>(m_hash)(std::get<Idx>(t)) + ...);
    }
    // [[no_unique_address]] std::tuple<std::hash<Ts>...> m_hash;
};

}  // namespace std




namespace leviathan
{

namespace mode
{
    struct fifo { };
    struct map { };
    struct hash { };
};


namespace detail
{


template <size_t MaxSize = 1000, typename F>
decltype(auto) memoized_invoke_fifo_impl(F&& f)
{
    using key = typename meta::function_traits<F>::args;
    using mapped = typename meta::function_traits<F>::return_type;
    std::list<std::pair<key, mapped>> table;

    return [=]<typename... Args>(Args&&... args) mutable
    {
        auto tuple = std::forward_as_tuple(std::forward<Args...>(args)...);
        auto cache = std::find_if(table.begin(), table.end(), [&](const auto& x)
        {
            return x.first == tuple;
        });

        if (cache == table.end())
        {
            auto result = std::apply(f, tuple);
            table.emplace_front(tuple, result);
        }
        else
        {
            // keep result front
            auto tmp = std::move(*cache);
            table.erase(cache);
            table.emplace_front(std::move(tmp));
        }
        while (table.size() > MaxSize) table.pop_back();
        return table.front().second;
    };
}

template <size_t MaxSize = 0, typename F> // dummy paras for MaxSize
decltype(auto) memoized_invoke_map_impl(F&& f)
{
    using key = typename meta::function_traits<F>::args;
    using mapped = typename meta::function_traits<F>::return_type;

#ifndef GNU_EXTEND_AVALIABLE
    std::map<key, mapped>
#else
    // splay tree has virtual function so it may slower, but I think for locality theory it may faster
    ::__gnu_pbds::tree<
        key, mapped, std::less<key>,
        ::__gnu_pbds::splay_tree_tag,
        ::__gnu_pbds::tree_order_statistics_node_update> 
#endif
                table;

    return [=]<typename... Args>(Args&&... args) mutable
    {
        auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
        auto cache = table.find(tuple);
        if (cache == table.end())
        {
            auto result = std::apply(f, tuple);
            table[tuple] = result;

            return result;
        }
        return cache->second;
    };
}

template <size_t MaxSize = 0, typename F> // dummy paras for MaxSize
decltype(auto) memoized_invoke_hash_impl(F&& f)
{
    using key = typename meta::function_traits<F>::args;
    using mapped = typename meta::function_traits<F>::return_type;

    std::unordered_map<key, mapped> table;
    return [=]<typename... Args>(Args&&... args) mutable
    {
        auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
        auto cache = table.find(tuple);
        if (cache == table.end())
        {
            auto result = std::apply(f, tuple);
            table[tuple] = result;
            return result;
        }
        return cache->second;
    };
}



} //  namespace detail




template <typename _Mode = mode::fifo, size_t MaxSize = 1000, typename Fn>
decltype(auto) memoized_invoke(Fn&& fn)
{
    class error_mode_exception : public std::exception
    {
    public:
        const char* what() const noexcept override 
        {
            return "supported mode: fifo, map";
        }
    };

    if constexpr (std::is_same_v<_Mode, mode::map>)
    {
        return detail::memoized_invoke_map_impl<MaxSize>(std::forward<Fn>(fn));
    }
    else if constexpr (std::is_same_v<_Mode, mode::fifo>)
    {
        return detail::memoized_invoke_fifo_impl<MaxSize>(std::forward<Fn>(fn));
    }
    else if constexpr (std::is_same_v<_Mode, mode::hash>)
    {
        return detail::memoized_invoke_hash_impl<MaxSize>(std::forward<Fn>(fn));
    }
    else
        throw error_mode_exception();
}





}  // namespace leviathan


#endif // __MEMORT_INVOKE_HPP__