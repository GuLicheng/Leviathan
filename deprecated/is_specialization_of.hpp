#pragma once

#include <type_traits>

namespace leviathan::meta
{

template <typename T, template <typename...> typename Primary>
struct is_specialization_of : std::false_type { };

template <template <typename...> typename Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type { };

template <typename T, template <typename...> typename Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

} // namespace leviathan::meta


