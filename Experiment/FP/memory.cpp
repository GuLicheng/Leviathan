// #include <lv_cpp/memory_invoke.hpp>
#include "base.hpp"
#include <map>
#include <tuple>

template <typename R, typename... Args>
auto make_memorized(R (*f)(Args...))
{
    std::map<std::tuple<Args...>, R> cache;   
    return [f, cache](Args... args) mutable {
        const auto arg_tuple = std::make_tuple(args...);
        const auto cached = cache.find(arg_tuple);
        if (cached == cache.end())
        {
            auto result = f(args...);
            cache[arg_tuple] = result;
            return result;
        }
        return cached->second;
    };
}

unsigned long long fib(unsigned long long n)
{
    return n < 2 ? n : fib(n - 1) + fib(n - 2);
}

int main()
{
    auto fibdemo = make_memorized(fib);
    std::cout << fibdemo(10) << '\n';
}

