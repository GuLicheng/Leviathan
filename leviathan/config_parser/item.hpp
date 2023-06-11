#include <leviathan/string/lexical_cast.hpp>
#include <string_view>
#include <optional>

namespace leviathan::config
{
    /**
     * @brief This value is used for storing the string of parser.
     * 
     * @param T The storage type of parser, maybe string or string_view.
    */
    template <typename T>
    struct basic_item
    {
        std::optional<T> m_value;

        template <typename U>
        std::optional<U> cast() const
        {
            if (!m_value)
                return std::nullopt;
            return leviathan::string::lexical_cast<U>(*m_value);
        }

    };

    using item = basic_item<std::string_view>;
}











