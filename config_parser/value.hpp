#include <string>
#include <concepts>

template <typename From, typename To> 
struct lexical_cast;

template <typename From, typename To>
    requires std::same_as<From, std::string> && std::is_arithmetic_v<To>
struct lexical_cast<From, To>
{
    To operator()(const From& src)
    {
        if constexpr (std::integral<To>)
            return (To)std::stoll(src);
        else
            return (To)std::stod(src);
    }
};


template <typename T>
class config_value
{
    T m_value;


    template <typename U>
    constexpr U as() const 
    {
        return lexical_cast<T, U>()(m_value);
    }

};









