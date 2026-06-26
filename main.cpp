#include <cassert>
#include <leviathan/extc++/annotation.hpp>
#include <leviathan/annotations/all.hpp>
#include <leviathan/extc++/meta.hpp>
#include <bits/stdc++.h>

// 前向声明一个枚举，用于测试嵌套作用域
enum class Color : uint8_t {
    Red = 0,
    Green,
    Blue,
    Yellow,
    Magenta,
    Cyan
};

struct DataSource
{
    template <typename T, typename Caster>
    static constexpr void operator()(std::optional<T>& opt, std::string name, Caster caster)
    {
        if constexpr (std::is_same_v<T, bool>) {
            opt.emplace(true);
        } else if constexpr (std::integral<T>) {
            opt.emplace(1);
        } else if constexpr (std::floating_point<T>) {
            opt.emplace(3.14);
        } else if constexpr (std::ranges::range<T>) {
            if constexpr (std::is_same_v<T, std::string>) {
                opt.emplace("Hello, World!");
            } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                opt.emplace(std::vector<int>{1, 2, 3});
            } else {
                opt.emplace();
            }
        } else if constexpr (std::is_enum_v<T>) {
            opt.emplace(static_cast<T>(0));
        } else {
            opt.emplace();
        }
    }
};

// 一个自定义的简单类，用于测试类类型
class CustomClass {
public:
    int id;
    std::string name;
    double value;

    // 默认构造函数
    CustomClass() : id(0), name(""), value(0.0) {}

    // 带参数的构造函数
    CustomClass(int id, const std::string& name, double value)
        : id(id), name(name), value(value) {}

    // 拷贝构造
    CustomClass(const CustomClass&) = default;
    // 移动构造
    CustomClass(CustomClass&&) = default;

    // 赋值操作符
    CustomClass& operator=(const CustomClass&) = default;
    CustomClass& operator=(CustomClass&&) = default;

    bool operator==(const CustomClass& other) const {
        return id == other.id && name == other.name && value == other.value;
    }

    bool operator<(const CustomClass& other) const {
        return id < other.id;
    }
};

// 测试用的主结构体
struct ComprehensiveTestStruct {
    // ===== 1. 基础整数类型 =====
    [[=cpp::refl::skip]]
    bool                bool_val;          // 布尔类型

    [[=cpp::refl::default_value('Y'), =cpp::refl::skip]]
    char                char_val;          // 字符类型（有符号）
    int                 int_val;
    double              double_val;

    // ===== 3. 枚举类型 =====
    Color               color_val;         // 前向声明的枚举
    enum class Weekday : uint8_t {         // 局部定义的枚举
        Monday = 0,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday
    } weekday_val;

    std::string         std_string_val;

    const char*         c_string_ptr;

    // ===== 5. 标准库容器 =====
    std::vector<int>                    vector_val;

    std::map<std::string, int>          map_val;

};

int main(int argc, char const *argv[])
{

    auto result = cpp::refl::construct_struct<ComprehensiveTestStruct>(DataSource{});
    
    assert(result.bool_val == false);
    assert(result.char_val == 'Y');
    assert(result.int_val == 1);
    assert(result.double_val == 3.14);
    assert(result.color_val == Color::Red);
    assert(result.weekday_val == ComprehensiveTestStruct::Weekday::Monday);

    return 0;
}

