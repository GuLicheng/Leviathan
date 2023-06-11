#include <leviathan/string/lexical_cast.hpp>
#include <string_view>

namespace leviathan::config_parser
{
    /**
     * @brief This value is used for storing the string of parser.
     * 
     * @param T The storage type of parser, maybe string or string_view.
    */
    template <typename T>
    struct basic_value
    {
        T m_value;

        template <typename U>
        auto cast() const
        {   
            return leviathan::string::lexical_cast<U>(m_value);
        }   
    };

    using value = basic_value<std::string_view>;
}











