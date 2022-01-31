#include "base.hpp"
#include <lv_cpp/string/opt.hpp>
#include <string_view>

template <typename String>
struct storage
{
    using type = std::conditional_t<std::is_lvalue_reference_v<String>, std::string_view, String>;
};

// std::views::concat -> lazy evaluation
// but concat may not friendly for const char* since it is not range
template <typename... Strings>
struct lazy_string_concat_helper
{

    std::tuple<Strings...> m_strs;

    lazy_string_concat_helper(Strings... s) : m_strs { std::move(s)... } { }

    operator std::string() const
    {
        return to_string();
    }

    std::string to_string() const 
    {
        // calculate length
        auto cal_len = []<typename... T>(const T&... t) {
            return (std::size(t) + ... + 0);
        };
        int len = std::apply(cal_len, m_strs);
        std::string result;
        result.reserve(len);
        // concat
        auto cat_str = [&]<typename... T>(const T&... t) {
            ((result += t), ...);
        };

        std::apply(cat_str, m_strs);
        return result;
    }

};

template <typename... Strings>
lazy_string_concat_helper(Strings&&...) -> 
    lazy_string_concat_helper<typename storage<Strings>::type...>;

int main()
{
    std::string s1 = "Hello";
    // for std::string("!"), it's better to use "!" instead.
    lazy_string_concat_helper concat { s1, " World ", std::string("!"), std::string_view{" C++ "} };
    std::cout << (std::string)concat << '\n';
}



