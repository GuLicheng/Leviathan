#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>

struct [[=cpp::derive::debug, =cpp::derive::from<cpp::json::value>]] FlattenValue
{
    int A;
    int B;
};

inline constexpr struct 
{
    template <typename T>
    static constexpr auto operator()(const T& value) 
    {
        return cpp::refl::function_value_annotation([x = *std::define_static_object(value)]() { return x; });
    }

    template <typename T>
    static constexpr auto operator()(std::initializer_list<T> values) 
    {
        return cpp::refl::function_array_annotation<T>(values);
    }
} DefaultValue;

union Union
{
    int X;
    double Y;

    constexpr Union() = default;

    template <typename T>
    constexpr Union(T x) {
        if (std::integral<T>) 
        {
            X = static_cast<int>(x);
        }
        else if (std::floating_point<T>)
        {
            Y = static_cast<double>(x);
        }
    }

};

struct [[=cpp::refl::lowercase, =cpp::derive::from<cpp::json::value>, =cpp::derive::debug]] Foo
{
    [[=cpp::refl::default_value(10)]]
    int X;

    [[=Union(4)]]
    [[=cpp::refl::skip]]
    [[=DefaultValue(cpp::array<cpp::array<int>>{ { 1, 2, 3}, { 4, 5, 6, 7}})]]
    std::vector<std::vector<int>> Y;
};




int main(int argc, char const *argv[])
{
    cpp::json::value v = {
        {"x", 120},
    };

    auto foo = cpp::cast<Foo>(v);

    std::println("foo: {}", foo);

    return 0;
}



