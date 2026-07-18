#pragma once

#include <leviathan/config_parser/json/value.hpp>
#include <leviathan/extc++/concepts.hpp>
#include <leviathan/extc++/functional.hpp>
#include <leviathan/type_caster.hpp>

#include <cassert>

namespace cpp::config::json
{
  
template <typename T>
class initializer
{
    const value& root;    

    static constexpr auto caster_adaptor = []<typename U>(std::optional<U>& opt, const auto& value)
    {
        opt.emplace(cpp::cast<U>(value));
    };

    static consteval std::meta::info constructor()
    {
        auto constructors = members_of(^^T, std::meta::access_context::current())
                          | std::views::filter(std::meta::is_constructor)
                          | std::views::filter(std::bind_back(cpp::refl::has_annotations, refl::constructor))
                          | std::ranges::to<std::vector>();
        return constructors[0];
    }  

    static constexpr bool allow_unknown_fields = !refl::has_annotations(^^T, refl::deny_unknown_fields);

    // Is necessary for [[=deny_unknown_fields]]?
    bool check_unknown_fields() const
    {
        if constexpr (allow_unknown_fields)
        {
            return true;
        }

        // Maybe not efficient, can we optimize it later if needed without changing followd implementation
        // in public part?

        // std::vector<json::string>
        auto names = root.as<object>()
                   | std::views::keys
                   | std::ranges::to<std::vector>();

        std::vector<std::string> names_of_fields;

        if constexpr (is_aggregate_type(^^T))
        {
            names_of_fields = refl::extract_field_names<T>();
        }
        else
        {
            constexpr static auto names = define_static_array(
                parameters_of(constructor()) | std::views::transform(std::meta::identifier_of)
            );
            // names_of_fields.append_range(names | std::views::transform([](auto name) { return std::string(name); }));
            names_of_fields.append_range(names | cpp::views::as<std::string>);
        }

        return std::ranges::all_of(names_of_fields, [&names](const auto& name) {
            return std::ranges::contains(names, name);
        });
    }

public:

    initializer(const value& root) : root(root) {}

    T operator()() const
    {
        if constexpr (is_aggregate_type(^^T))
        {
            return aggregate_constructor();
        }
        else
        {
            return user_defined_constructor();
        }
    }

    T user_defined_constructor() const
    {
        constexpr auto ctor = constructor();
        constexpr static auto params = define_static_array(parameters_of(ctor));
        constexpr auto [...indices] = std::make_index_sequence<params.size()>();

        auto impl = [&]<size_t Idx>() {
            using ParamType = typename [:remove_cvref(type_of(params[Idx])):];
            auto ParamName = identifier_of(params[Idx]);
            return cpp::cast<ParamType>(root.as<object>().find(string(ParamName))->second);
        };

        auto result = T(impl.template operator()<indices>()...);
        
        if (!cpp::refl::check_field(result))
        {
            throw std::runtime_error(std::format("Field check failed for {}", std::string(identifier_of(^^T))));
        }

        return result;
    }

    T aggregate_constructor() const
    {
        constexpr auto ctx = std::meta::access_context::current();
        
        // base class
        constexpr auto bases = define_static_array(bases_of(^^T, ctx));
        constexpr auto M = bases.size();
        constexpr auto [...base_indices] = std::make_index_sequence<M>();

        // current
        constexpr auto members = define_static_array(nonstatic_data_members_of(^^T, ctx));
        constexpr auto N = members.size();
        constexpr auto [...indices] = std::make_index_sequence<N>();

        auto result = T(
            initializer<typename [:type_of(bases[base_indices]):]>(root)()...,
            initialize_field<members[indices]>()...
        );

        if (!cpp::refl::check_field(result))
        {
            throw std::runtime_error(std::format("Field check failed for {}", std::string(identifier_of(^^T))));
        }

        return result;
    }

    template <std::meta::info Field>
    auto initialize_field() const
    {
        using FieldType = typename [:type_of(Field):];
        std::optional<FieldType> result = std::nullopt;
        
        if constexpr (!refl::has_annotations(Field, refl::skip_deserialization, refl::skip))
        {
            if constexpr (refl::has_annotations(Field, refl::flatten))
            {
                result.emplace(initializer<FieldType>(root)());
            }
            else
            {
                auto names = refl::handle<Field>::identifier_and_aliases();
                auto it = root.as<object>().end();
                
                for (auto name : names)
                {
                    it = root.as<object>().find(name);

                    if (it != root.as<object>().end())
                    {
                        break;
                    }
                }
                

                if (it != root.as<object>().end())
                {
                    constexpr auto info = refl::select_annotation_with_type(^^caster_adaptor, Field, ^^refl::serializer_annotation);
                    std::invoke(extract<typename [:type_of(info):]>(info), result, it->second);
                }
                else if (refl::has_annotations(Field, refl::required))
                {
                    throw std::runtime_error(std::format("Field {} is required but not found in the JSON object", (display_string_of(Field))));
                }
            }
        }

        if (!result)
        {
            result = refl::handle<Field>::default_value();
        }

        return *result;
    }
    
};

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
        return initializer<T>(root)();
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
            // return encoder()(v);
            return Range(v.as<string>().begin(), v.as<string>().end());
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

template <typename Enum>
struct enum_caster
{
    static Enum operator()(const value& v)
    {
        if (v.is<string>())
        {
            return enum_str_decoder<Enum>()(v.as<string>());
        }
        else if (v.is<number>() && v.as<number>().is_integer())
        {
            using UnderlyingType = std::underlying_type_t<Enum>;
            const auto n = v.as<number>().as<UnderlyingType>();
            return enum_int_decoder<Enum>()(n);
        }
        throw std::runtime_error("Value is not a string or integer for enum");
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
        else if constexpr (std::is_enum_v<T> && refl::has_annotations(^^T, cpp::derive::from<value>))
        {
            return enum_caster<T>::operator()(v);
        }
        else if constexpr (std::is_class_v<T> && refl::has_annotations(^^T, cpp::derive::from<value>))
        {
            return universal_caster<T>::operator()(v);
        }
        else
        {
            static_assert(false, "No caster available for this type");
        }
    }
};

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
                m_result += indent() + std::format(R"("{}":)", it->first);

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

// Cast json value to c++ type, may not perfect
template <typename Target>
struct cpp::optional_caster<cpp::json::value, Target>
{
    static std::optional<Target> operator()(const cpp::json::value& v)
    {
        try
        {
            auto result = cpp::json::detail::caster<Target>::operator()(v);
            return std::make_optional(std::move(result));
        }
        catch (...)
        {
            return std::nullopt;
        }
    }
};
