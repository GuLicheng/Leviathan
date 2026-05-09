#pragma once

#include <meta>
#include <type_traits>
#include <string_view>
#include <stdexcept>
#include <string>
#include <vector>
#include <leviathan/annotations.hpp>

namespace cpp::refl
{
    
template <typename Enum>
constexpr std::vector<std::pair<Enum, std::string>> get_enum_mapping()
{
    std::vector<std::pair<Enum, std::string>> mapping;

    template for (constexpr auto e : define_static_array(enumerators_of(^^Enum))) 
    {
        auto name = std::string(identifier_of(e));
        
        template for (constexpr auto anno : define_static_array(annotations_of(e)))
        {
            using AnnoType = typename [:type_of(anno):];

            if constexpr (std::is_base_of_v<cpp::refl::rename_annotation, AnnoType>)
            {
                name = std::invoke(extract<AnnoType>(anno), name);
            }
            else if constexpr (std::is_base_of_v<cpp::refl::ignore_annotation, AnnoType>)
            {
                name = "<ignored>";
                break;
            }
        }
        mapping.emplace_back(std::make_pair((Enum)[:e:], name));
    }
    return mapping;
}

} // namespace cpp::refl

