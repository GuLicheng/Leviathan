#include <meta>
#include <format>
#include <string>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <generator>
#include <leviathan/extc++/tuple.hpp>

inline constexpr struct { } TestFunction;
inline constexpr struct { } DataGenerator;
 
template <typename T>
struct Span
{
    const T* data;

    size_t size;

    consteval Span(std::initializer_list<T> init) : data(define_static_array(init).data()), size(init.size()) { }

    template <std::ranges::range R>
    constexpr operator R() const { return R(data, data + size); }
};

struct [[=DataGenerator]] ComplexObjectGenerator
{
    using R = cpp::tuple<std::vector<std::string>>;

    static std::generator<R> operator()() 
    {
        co_yield cpp::make_tuple(std::vector<std::string>{"Hello", "World"});
        co_yield cpp::make_tuple(std::vector<std::string>{"Foo", "Bar"});
    }
};

namespace TestNamespace
{

[[=TestFunction]]
void Func1() { std::println("Func1 called"); }

[[=TestFunction]]
[[=InlineData(42, 3.14, true, "Hello")]]
[[=InlineData(1, 2.17, false, "World")]]
void Func2(int a, double b, bool c, std::string d) 
{
    std::println("Func2 called with: {}, {}, {}, {}", a, b, c, d);
}


[[=TestFunction]]
[[=InlineData(Span{1, 2, 3})]]
void Func3(std::vector<int> vec) 
{
    std::println("Func3 called with vector: {}", vec);
}

[[=TestFunction]]
[[=ComplexObjectGenerator()]]
void Func4(std::vector<std::string> vec)
{
    std::println("Func4 called with vector: {}", vec);
} 

}  // namespace TestNamespace

template <std::meta::info Info>
consteval bool IsTuple()
{
    constexpr auto type = type_of(Info);
    return has_template_arguments(type) && template_of(type) == ^^cpp::tuple; 
}

template <std::meta::info Namespace>
void InvokeTestFunctions()
{
    static_assert(is_namespace(Namespace), "Template parameter must be a namespace");
    constexpr auto ctx = std::meta::access_context::current();

    template for (constexpr auto info : define_static_array(members_of(Namespace, ctx))) 
    {
        if constexpr (is_function(info) && cpp::refl::has_annotation(info, TestFunction)) 
        {
            constexpr auto params_count = parameters_of(info).size();

            if constexpr (params_count == 0) 
            {
                std::invoke([:info:]);
            } 
            else 
            {
                template for (constexpr auto anno : define_static_array(annotations_of(info))) 
                {
                    if constexpr (IsTuple<anno>()) 
                    {
                        constexpr auto args = extract<typename [:type_of(anno):]>(anno);
                        cpp::apply([:info:], args);
                    } 
                    else if constexpr (cpp::refl::has_annotation(type_of(anno), DataGenerator)) 
                    {
                        for (auto data : std::invoke(extract<ComplexObjectGenerator>(anno))) 
                        {
                            cpp::apply([:info:], std::move(data));
                        }
                    }
                }
            }
        }
    }
}

template <typename T, size_t N>
using constant_array = const T[N];

int main(int argc, char const *argv[]) 
{
    InvokeTestFunctions<^^TestNamespace>();

    std::println("Buffer: {}", constant_array<int, 10>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

    return 0;
}

