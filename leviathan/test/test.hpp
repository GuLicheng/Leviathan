/*

    - Terminal output coloring and formatting

    - Logging and debugging utilities

    - Global state management for tests

*/
#pragma once

#include <format>
#include <vector>
#include <source_location>
#include <leviathan/extc++/annotation.hpp>
#include <leviathan/extc++/meta.hpp>
#include <leviathan/extc++/tuple.hpp>

namespace cpp::test
{

template <typename... Ts>
consteval auto inline_data(const Ts&... ts)
{
    return cpp::make_tuple(*std::define_static_object(ts)...);
}

template <std::meta::info Namespace>
void invoke_test_functions()
{
    static_assert(is_namespace(Namespace), "Template parameter must be a namespace");
    constexpr auto ctx = std::meta::access_context::current();

    template for (constexpr auto info : define_static_array(members_of(Namespace, ctx))) 
    {
        if constexpr (is_function(info) && cpp::refl::has_annotation(info, cpp::refl::test)) 
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
                    if constexpr (cpp::refl::instance_of_template<anno, ^^cpp::tuple>())
                    {
                        constexpr auto args = extract<typename [:type_of(anno):]>(anno);
                        cpp::apply([:info:], args);
                    } 
                    else if constexpr (cpp::refl::has_annotation(type_of(anno), cpp::refl::range_maker)) 
                    {
                        for (auto data : std::invoke(extract<typename [:type_of(anno):]>(anno))) 
                        {
                            cpp::apply([:info:], std::move(data));
                        }
                    }
                }
            }
        }
    }
}


}  // namespace cpp::test