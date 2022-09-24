
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <algorithm>
#include <spanstream>

#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/ranges.hpp>
#include <lv_cpp/bind_back.hpp>

template <typename Range>
void print_range(Range&& range)
{
    std::cout << "[";
    using value_type = std::ranges::range_value_t<std::remove_reference_t<Range>>;
    std::ranges::copy(range, std::ostream_iterator<value_type>(std::cout));
    std::cout << "]\n";
}

int main()
{
    std::vector values = { 1, 2, 3, 4 };
    auto result1 = values
            | std::views::transform([](int x) { return std::to_string(x); })
            | leviathan::ranges::join_with(' ');

    auto result2 = values
            | leviathan::ranges::concat_with(leviathan::ranges::repeat(-1, 2));

    print_range(result1);
    print_range(result2);
}

