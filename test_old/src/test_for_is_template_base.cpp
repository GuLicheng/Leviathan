#include <type_traits>
#include <tuple>


/*  
    Some bug to be fixed
*/
template <template <typename...> typename BaseTemplateClass, typename DerivedInstance, typename = std::void_t<>>
struct is_instance_base_of : std::false_type { };

template <template <typename...> typename BaseTemplateClass, typename DerivedInstance>
struct is_instance_base_of<BaseTemplateClass, DerivedInstance, std::void_t<decltype([]<typename... Args>(BaseTemplateClass<Args...>*){}(std::declval<DerivedInstance*>()))>>
    : std::true_type { };

template <template <typename...> typename BaseTemplateClass, typename DerivedInstance>
constexpr auto is_instance_base_of = is_instance_base_of<BaseTemplateClass, DerivedInstance>::value;


template <template <typename ...> typename B, typename T, typename = std::void_t<>>
struct is_derived : std::false_type
{
};

template <template <typename ...> typename B, typename T>
struct is_derived <B, T, std::void_t<decltype([]<typename... Args>(B<Args...>*) {}(std::declval<T*>())) >> : std::true_type
{
};

template <template <typename ...> typename Base, typename T>
inline constexpr auto is_derived_v = is_derived<T, Base>::value;

template <typename... Args>
struct base
{
};

struct A : base<int>, base<double>
{
};

template <typename T>
struct B : public base<T>
{
};

int main(int argc, char* argv[])
{
    static_assert(is_derived_v<base, A>);
    static_assert(is_derived_v<base, base<int>>);
    static_assert(is_derived_v<base, std::tuple<int>>);
    static_assert(is_derived_v<base, std::tuple<>>);
}