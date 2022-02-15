#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <string>
#include <lv_cpp/meta/template_info.hpp>

template <typename T, typename... BaseTypes>
using Variable = std::variant<std::vector<T>, std::map<std::variant<BaseTypes...>,  T>, BaseTypes...>;

template <typename T>
using Val = Variable<T, int, bool, double, std::string>;

template <template <typename> typename F>
struct FixPoint : F<FixPoint<F>> 
{
    using F<FixPoint<F>>::F;
};

using Json = FixPoint<Val>;
using JsonList = std::vector<Json>;
using JsonDict = std::map<std::variant<int, bool, double, std::string>, Json>;
using Class = std::map<std::string, Json>;


template <typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::variant<Ts...>& v)
{
    return std::visit([&](const auto& x) -> auto& { return os << x; }, v);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    os << '[';
    auto first = v.begin(), last = v.end();
    for (auto iter = first; iter != last; ++iter)
    {
        if (iter != first) os << ", ";
        os << *iter;
    }
    return os << ']';
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& v)
{
    os << '{';
    auto first = v.begin(), last = v.end();
    for (auto iter = first; iter != last; ++iter)
    {
        if (iter != first) os << ", ";
        os << *iter;
    }
    return os << '}';
}

template <typename F, typename S>
std::ostream& operator<<(std::ostream& os, const std::pair<F, S>& v)
{
    return os << '(' << v.first << ": " << v.second << ')';
}


int main()
{
    Json config1 {
        JsonList { 1, true, "Hello", JsonDict { {"42", 42}, {42, "42"} } }
    };
    Json config2 { true };
    Json config3 { 1 };
    Json config4 { "Hello World" };
    Json config5 = { JsonDict { { "name", "Alice"}, { "age", 18 } } };
    std::cout << config1 << '\n';
    std::cout << config5 << '\n';
}
