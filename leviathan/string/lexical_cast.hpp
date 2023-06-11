#pragma once

#include <type_traits>

namespace leviathan::string
{
    template <typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template <typename Target, typename Source>
    struct lexical_cast_t;

    /**
     * @brief Cast Source to Target.
     * @return Any type. We use decltype(auto) since if Target is same as Source, this function
     *  just return the reference of source itself. But for some arithmetic type, it may return
     *  std::optional<Target> which used for replace exception.
     * @exception Any exception that lexical_cast_t will throw.
    */
    template <typename Target, typename Source, typename... Args>
    decltype(auto) lexical_cast(const Source& source, Args&&... args)
    { return lexical_cast_t<Target, Source>()(source, (Args&&) args...); }

    template <typename Target>
    struct lexical_cast_t<Target, Target>
    {
        constexpr static const Target& operator()(const Target& s) 
        {
            return s;
        }
    };
} // namespace leviathan

