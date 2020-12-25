#ifndef _CONCEPTS_EXTEND_HPP_
#define _CONCEPTS_EXTEND_HPP_

#include "type_list.hpp"
#include <string>
#include <string_view>
#include <concepts>


#define CreateTemplateConceptsHelper(is, TypeName, v, ConceptName, NameSpace, InnerNameSpace)            \
    namespace InnerNameSpace                                                                             \
    {                                                                                                    \
    template <typename T> struct is##TypeName : std::false_type { };                                     \
    template <typename... Ts> struct is##TypeName < NameSpace :: TypeName <Ts...>> : std::true_type { }; \
    }                                                                                                    \
    template <typename T>                                                                                \
    inline constexpr bool is##TypeName##v = InnerNameSpace :: is##TypeName <std::remove_cvref_t<T>>::value;  \
    template <typename T>                                                                                \
    concept ConceptName = is##TypeName##v <T>
    

#define CreateTemplateConcepts(TypeName, ConceptName, NameSpace) \
    CreateTemplateConceptsHelper(is_ , TypeName, _v, ConceptName, NameSpace, detail)

// for instance: CreateTemplateConcepts(tuple, tuple_concept, ::std)
// tuple is template class name
// tuple_concepts is user-defind name
// ::std::tuple is in ::std domain

// helper / detail namespace 
namespace leviathan {

namespace detail {

// basic_string ... begin
template <typename T>
struct is_basic_string : std::false_type { };

template <typename T>
struct is_basic_string<std::basic_string<T>> : std::true_type { };

template <typename T>
inline constexpr bool is_basic_string_v = is_basic_string<std::remove_cvref_t<T>>::value;

template <typename T>
concept std_basic_string = is_basic_string_v<T>;
// basic_string ... end


// basic_string_view ...  begin

template <typename T>
struct is_basic_string_view : std::false_type { };

template <typename T>
struct is_basic_string_view<std::basic_string_view<T>> : std::true_type { };

template <typename T>
inline constexpr bool is_basic_string_view_v = is_basic_string_view<std::remove_cvref_t<T>>::value;

template <typename T>
concept std_basic_string_view = is_basic_string_view_v<T>;

// basic_string_view ... end

// print STL container 
template <typename T>
concept container_include_string = requires (T t) {
    // typename std::decay_t<T>::iterator;
    // typename std::decay_t<T>::value_type;
    std::cbegin(t);
    std::cend(t);
};


template <typename T> // without string and string_view
concept container = container_include_string<T> && !is_basic_string_v<T> && !is_basic_string_view_v<T>;

} // namespace detail


} //  namespace leviathan




#endif