#include <ranges>
#include <functional>

namespace leviathan
{
    struct less 
    {
        template <typename L, typename R>
            requires std::totally_ordered_with<L, R>
        constexpr bool operator()(L&& l, R&& r) const
        noexcept(noexcept(std::declval<L>() < std::declval<R>()))
        {
            return (L&&)l < (R&&)r;
        }

        using is_transparent = void;
    };
}






