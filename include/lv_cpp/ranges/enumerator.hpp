#pragma

#include <lv_cpp/ranges/zip.hpp>


namespace leviathan
{

namespace views
{

inline constexpr auto enumerator = []<typename... Ranges>(Ranges&&... rgs)
{
    auto iota_view = ::std::views::iota(0);
    return ::leviathan::views::zip(std::move(iota_view), ::std::forward<Ranges>(rgs)...);
};

}



}