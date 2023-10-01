#pragma once

#include "toml_value.hpp"
#include "json_value.hpp"

#include <leviathan/meta/template_info.hpp>

namespace leviathan::config
{
    namespace detail
    {
        using toml::toml_value;
        using json::json_value;

        template <typename T>
        inline constexpr bool is_unique_ptr = is_specialization_of_v<T, std::unique_ptr>;

        template <typename T>
        inline constexpr bool is_shared_ptr = is_specialization_of_v<T, std::shared_ptr>;
    
        template <typename T>
        inline constexpr bool is_pointer_v = 
                            is_unique_ptr<T>
                         || is_shared_ptr<T>
                         || std::is_pointer_v<T>;

        struct convert_helper
        {
            json_value operator()(const toml_value& tv) const
            {
                return std::visit([this]<typename T>(const T& val) -> json_value {
                    if constexpr (is_pointer_v<T>)
                    {
                        auto ptr = std::to_address(val);
                        return this->operator()(*ptr);
                    }
                    else
                    {
                        return this->operator()(val);
                    }
                }, tv.data());
            }

            json_value operator()(const toml::toml_boolean& b) const
            { return json::json_boolean(b);}

            json_value operator()(const toml::toml_array& arr) const
            { 
                json::json_array ja;

                ja.reserve(arr.size());

                std::transform(arr.begin(), arr.end(), std::back_inserter(ja), *this);

                return json_value(std::move(ja));
            }

            json_value operator()(const toml::toml_string& str) const
            { return json::json_string(str); }

            json_value operator()(const toml::toml_datetime& str) const
            { return json::json_string(str.to_string()); }

            json_value operator()(const toml::toml_float& num) const
            { return json::json_number(num); }

            json_value operator()(const toml::toml_integer& num) const
            { return json::json_number(num); }

            json_value operator()(const toml::toml_table& tt) const
            {
                json::json_object obj;

                std::for_each(tt.begin(), tt.end(), [&, this](const auto& pa) {
                    obj.emplace(pa.first, this->operator()(pa.second));
                });
                
                return obj;
            }
        };
    }

    json::json_value toml2json(const toml::toml_value& tv)
    { return detail::convert_helper()(tv); }
}

namespace leviathan
{
    using config::toml2json;
}