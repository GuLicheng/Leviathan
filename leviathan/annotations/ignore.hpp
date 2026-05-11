#pragma once

#include <leviathan/annotations/common.hpp>

namespace cpp::refl
{

struct ignore_annotation : annotation { };

inline constexpr auto ignore = ignore_annotation{};

template <std::meta::info Info>
consteval bool is_ignored()
{
    static_assert(has_identifier(Info), "Info must have an identifier");

    template for (constexpr auto anno : define_static_array(annotations_of(Info)))
    {
        using AnnoType = typename [:type_of(anno):];

        if constexpr (std::is_base_of_v<ignore_annotation, AnnoType>)
        {
            return true;
        }
    }
    return false;
}

}
