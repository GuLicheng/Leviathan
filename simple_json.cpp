#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>
#include <leviathan/extc++/format.hpp>
#include <leviathan/extc++/variant.hpp>
#include <leviathan/extc++/array.hpp>
#include <leviathan/config_parser/json/json.hpp>
#include <print>
#include <ranges>
#include <iostream>
#include <unordered_map>

class JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonString = std::string;
using JsonObject = std::unordered_map<JsonString, JsonValue>;
using JsonNull = std::nullptr_t;
using JsonBoolean = bool;
using JsonNumber = double;

class JsonValue : public std::variant<JsonNull, JsonNumber, JsonBoolean, JsonString, JsonArray, JsonObject>
{
public:
    using base = std::variant<JsonNull, JsonNumber, JsonBoolean, JsonString, JsonArray, JsonObject>;
    using base::base;
}; 

template <>
struct std::formatter<JsonValue> : cpp::tag_union_formatter
{
};

template <typename T> struct JsonCaster;

template <> 
struct JsonCaster<bool> 
{
    static bool operator()(const JsonValue& value)
    {
        return std::get<JsonBoolean>(value);
    }
};

template <typename T> 
    requires (std::meta::is_arithmetic_type(^^T))
struct JsonCaster<T>
{
    static T operator()(const JsonValue& value)
    {
        return static_cast<T>(std::get<JsonNumber>(value));
    }
};

template <std::ranges::input_range R>
struct JsonCaster<R>
{
    static R operator()(const JsonValue& value)
    {
        using ValueType = std::remove_cv_t<std::remove_reference_t<std::ranges::range_value_t<R>>>;

        if constexpr (std::is_same_v<ValueType, char>)
        {
            // JsonString
            return R(std::get<JsonString>(value).begin(), std::get<JsonString>(value).end());
        }
        else if constexpr (cpp::meta::pair_like<ValueType>)
        {
            // JsonObject
            using KeyType = std::remove_cvref_t<std::tuple_element_t<0, ValueType>>;
            using MappedType = std::tuple_element_t<1, ValueType>;

            return std::get<JsonObject>(value)
                | cpp::views::pair_transform(JsonCaster<KeyType>(), JsonCaster<MappedType>())
                | std::ranges::to<R>();
        }
        else
        {
            // JsonValue
            return std::get<JsonArray>(value) 
                 | std::views::transform(JsonCaster<ValueType>()) 
                 | std::ranges::to<R>();
        }
    }
};

inline constexpr struct
{
    template <typename T>
    static auto operator()(const JsonValue& value) 
    {
        return JsonCaster<T>()(value);
    }
} json_caster;

int main() {

    JsonValue json_value = JsonObject {
        {"name", JsonString("John")},
        {"age", JsonNumber(30)},
        {"is_student", JsonBoolean(false)},
        {"courses", JsonArray{JsonString("Math"), JsonString("Science")}},
        {"address", JsonObject{
            {"street", JsonString("123 Main St")},
            {"city", JsonString("Anytown")},
            {"zip", JsonNumber(12345)}
        }},
        {"null_value", JsonNull(nullptr)}
    };

    std::cout << std::format("{}", json_value) << std::endl;

    std::println("Name: {}", JsonCaster<std::vector<std::string>>()(std::get<JsonObject>(json_value)["courses"]));

}
