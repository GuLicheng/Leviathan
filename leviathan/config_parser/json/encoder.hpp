#pragma once

#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/extc++/functional.hpp>
#include <leviathan/type_caster.hpp>

#include <cassert>

namespace cpp::config::json
{
    
namespace detail
{

struct encoder
{
    static std::string operator()(const number& num)
    {
        return std::visit(to_string, num.data());
    }

    static std::string operator()(const string& str) 
    {
        return std::format("\"{}\"", str);
    }

    static std::string operator()(const array& arr) 
    {
       auto context = arr 
                    | cpp::views::transform_join_with(encoder(), ',')
                    | std::ranges::to<std::string>();
        return std::format("[{}]", context);
    }

    static std::string operator()(const boolean& b) 
    {
        return b ? "true" : "false";
    }

    static std::string operator()(const null&) 
    {
        return "null";
    }

    static std::string operator()(const object& obj) 
    {
        auto kv2string = [=]<typename PairLike>(PairLike&& kv) 
        {
            return std::format("{}:{}",
                encoder()(std::get<0>((PairLike&&)kv)), 
                encoder()(std::get<1>((PairLike&&)kv))
            );
        };

        auto context = obj
                     | cpp::views::transform_join_with(kv2string, ',')
                     | std::ranges::to<std::string>();
        return std::format("{{{}}}", context);
    }

    static std::string operator()(const value& v) 
    {
        return std::visit([]<typename T>(const T& x) {
            return encoder::operator()(value::accessor()(x));
        }, v.data());
    }
};

template <typename T> 
struct caster;

template <typename T>
struct universal_caster
{
    static T operator()(const value& root)
    {
        alignas(T) char buffer[sizeof(T)];
        T& obj = *reinterpret_cast<T*>(buffer);

        constexpr auto ctx = std::meta::access_context::current();

        template for (constexpr auto mem : define_static_array(nonstatic_data_members_of(^^T, ctx))) 
        {
            auto name = cpp::refl::extract_name_by_annotation<mem, ^^T>();
            auto field = cpp::json::string(name);

            auto it = root.as<cpp::json::object>().find(field);

            if (it == root.as<cpp::json::object>().end()) 
            {
                bool initialized = false;

                template for (constexpr auto anno : define_static_array(annotations_of(mem)))
                {
                    using AnnoType = typename [:type_of(anno):];
                        
                    // FIXME
                    if constexpr (std::is_base_of_v<cpp::refl::value_annotation<typename [:type_of(mem):]>, AnnoType>)
                    {
                        constexpr auto default_value = std::invoke(extract<AnnoType>(anno));
                        std::construct_at(std::addressof(obj.[:mem:]), default_value); // construct with default value
                        initialized = true;
                        break;
                    }
                }

                if (!initialized)
                {
                    std::construct_at(std::addressof(obj.[:mem:]), typename [:type_of(mem):]{}); 
                }
            }
            else
            {
                std::construct_at(  
                    std::addressof(obj.[:mem:]), 
                    cpp::cast<typename [:type_of(mem):]>(it->second)
                );
            }

            // Check field is valid
            template for (constexpr auto anno : define_static_array(annotations_of(mem)))
            {
                using AnnoType = typename [:type_of(anno):];

                // if constexpr (std::is_base_of_v<cpp::refl::choice_annotation, AnnoType>)
                if constexpr (refl::has_annotation(type_of(anno), cpp::refl::choice_annotation))
                {
                    if (!std::invoke(extract<AnnoType>(anno), obj.[:mem:]))
                    {
                        throw std::runtime_error(std::format("Field {} has invalid value", name));
                    }
                }
            }

        }

        return std::move(obj);
    }
};

struct boolean_caster
{
    static bool operator()(const value& v)
    {
        return v.is<boolean>() ? v.as<boolean>() : throw std::runtime_error(std::format("Value is not a boolean, but {}", v.type_name()));
    }
};

template <typename Arithmetic>
struct arithmetic_caster
{
    static Arithmetic operator()(const value& v)
    {
        if (v.is<number>())
        {
            return v.as<number>().as<Arithmetic>();
        }
        else if (v.is<boolean>())
        {
            return v.as<boolean>() ? Arithmetic(1) : Arithmetic(0);
        }
        else if (v.is<string>())
        {
            std::string_view ctx = v.as<string>();
            auto result = cast_optional<Arithmetic>(ctx);

            if (result)
            {
                return *result;
            }
            else
            {
                throw std::runtime_error("Failed to convert string to number" + std::string(ctx));
            }
        }
        else
        {
           throw std::runtime_error("Value is not a number");
        }
    }
};

template <typename Range>
struct range_caster
{
    static Range operator()(const value& v)
    {
        if constexpr (meta::string_like<Range>)
        {
            return encoder()(v);
        }
        else
        {
            using ValueType = typename Range::value_type;

            if constexpr (cpp::meta::pair_like<ValueType>)
            {
                // For map<K, V>, the value type is std::pair<const K, V>
                // we should remove cv-qualifiers for value_type::first_type.
                // We use std::tuple_element to get the first and second types
                // instead of typename Container::key_type, it will make
                // std::vector<std::pair<K, V>> work as well.
                using KeyType = std::remove_cvref_t<std::tuple_element_t<0, ValueType>>;
                using MappedType = std::tuple_element_t<1, ValueType>;
                
                if (v.is<object>())
                {
                    return v.as<object>()
                        | cpp::views::pair_transform(cpp::cast<KeyType>, cpp::cast<MappedType>)
                        | std::ranges::to<Range>();
                }
                else
                {
                    throw std::runtime_error("Value is not an object");
                }
            }
            else
            {
                // array
                if (v.is<array>())
                {
                    return v.as<array>() 
                        | std::views::transform(caster<ValueType>()) 
                        | std::ranges::to<Range>();
                }
                else
                {
                    throw std::runtime_error("Value is not an array");
                }
            }
        }
    }
};

template <typename T> 
struct caster
{
    static T operator()(const value& v)
    {
        if constexpr (std::same_as<bool, T>)
        {
            return boolean_caster::operator()(v);
        }
        else if constexpr (cpp::meta::arithmetic<T>)
        {
            return arithmetic_caster<T>::operator()(v);
        }
        else if constexpr (std::ranges::range<T>)
        {
            return range_caster<T>::operator()(v);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            return enum_decoder<T>()(v.as<string>());
        }
        else if constexpr (use_default_caster<T>)
        {
            return universal_caster<T>::operator()(v);
        }
        else
        {
            static_assert(false, "No caster available for this type");
        }
    }
};



/*
{
    "name": "Alice",
    "age": 30,
    "is_student": false,
    "grades": [
        85,
        90,
        78
    ],
    "address": {
        "street": "123 Main St",
        "city": "Wonderland",
        "zip": "12345"
    }
} 
*/

class indented_encoder
{
    struct impl
    {
        std::string m_result;
        int m_level = 0;
        int m_count;

        impl(int count) : m_count(count) {}

        std::string indent() const
        {
            return std::string(m_count * m_level, ' ');
        }

        void operator()(const number& number)
        {
            m_result += std::visit(cpp::to_string, number.data());
        }

        void operator()(const string& str) 
        {
            m_result += std::format("\"{}\"", str);
        }

        void operator()(const array& arr) 
        {
            m_result += "[\n";
            m_level++;

            for (std::size_t i = 0; i < arr.size(); ++i)
            {
                m_result += indent();
                this->operator()(arr[i]);

                if (i != arr.size() - 1) 
                {
                    m_result.append(",\n");
                }
            }

            m_result += "\n";
            m_level--;
            m_result += indent() + "]";
        }

        void operator()(const boolean& boolean) 
        {
            m_result.append((boolean ? "true" : "false"));
        }

        void operator()(const null&) 
        {
            m_result.append("null"); 
        }

        void operator()(const object& object) 
        {
            m_result += "{\n";

            auto size = object.size();
            size_t idx = 0;
            m_level++;

            for (auto it = object.begin(); it != object.end(); ++it, idx++)
            {
                m_result += indent() + std::format(R"("{}" : )", it->first);

                this->operator()(it->second);

                if (idx != size - 1) 
                {
                    m_result += ",\n";
                }
            }
            
            m_result += "\n";
            m_level--;
            m_result += indent() + "}";
        }

        void operator()(const value& value) 
        {
            std::visit([this]<typename T>(const T& x) {
                this->operator()(value::accessor()(x));
            }, value.data());
        }
    };

public:

    static auto operator()(const value& x, int indent)
    {
        impl encoder(indent);
        encoder(x);
        return std::move(encoder.m_result);
    }
};

}  // namespace detail

inline constexpr struct
{
    static std::string operator()(const value& x, int indent = 0)
    {
        using NoneEncoder = detail::encoder;
        using IndentedEncoder = detail::indented_encoder;

        if (indent == 0)
        {
            return NoneEncoder()(x);
        }
        else
        {
            return IndentedEncoder()(x, indent);
        }
    }
} dumps;

inline constexpr struct
{
    static void operator()(const value& x, const char* filename, int indent = 0) 
    {
        auto context = dumps(x, indent);
        write_file(context, filename);
    }
} dump;

} // namespace cpp::config::json

// https://blog.csdn.net/jkddf9h8xd9j646x798t/article/details/127954236
// template <typename CharT>
template <>
struct std::formatter<cpp::json::value, char> 
{
    // enum format_kind { indented, none };

    template <typename ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx)
    {
        auto symbol = std::ranges::find(ctx.begin(), ctx.end(), '}');
        std::string_view fmt = std::string_view(ctx.begin(), symbol);
        m_indent = fmt.empty() ? 0 : cpp::cast<int>(fmt);

        // assert(m_indent >= 0 && m_indent <= 8 && "Indentation level must be between 0 and 8");
        m_indent = std::clamp(m_indent, 0, 8);
        return symbol; // return the end iterator
    }

    template <typename FmtContext>
    typename FmtContext::iterator format(const cpp::json::value& value, FmtContext& ctx) const
    {
        auto result = cpp::json::dumps(value, m_indent);        
        return std::ranges::copy(result, ctx.out()).out;
    }   

private:

    int m_indent = 0;
};

// Cast json value to c++ type
template <typename Target>
struct cpp::optional_caster<cpp::json::value, Target>
{
    static auto operator()(const cpp::json::value& v)
    {
        auto result = cpp::json::detail::caster<Target>::operator()(v);
        return std::make_optional(std::move(result));
    }
};
