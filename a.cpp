#include <numeric>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>
#include <format>

enum class option_type
{
    only_one,
    optional,
    at_least_one,
};

struct option
{
    bool m_required;
    std::string m_longname;
    std::string m_shortname;
    option_type m_type; 
}

struct options
{
    std::vector<option> m_options;
    options* m_next;
}

struct option_builder
{
    options* m_link;

    option_builder& required(bool r = true)
    {
        m_required = r;
        return *this;
    }

    option_builder& longname(std::string ln)
    {
        m_longname = std::move(ln);
        return *this;
    }

    option_builder& shortname(std::string sn)
    {
        m_shortname = std::move(sn);
        return *this;
    }

    
};




option make_option()
{
    return option();
}

std::span<const char*> ToArgs(int argc, const char* argv[])
{
    std::span<const char*> args(argv, argc);
    std::ranges::copy(std::ranges::subrange(argv, argv + argc), args.begin());
    return args;
}

int main(int argc, char const *argv[])
{
    auto args = ToArgs(argc, argv);

    std::ranges::copy(args, std::ostream_iterator<const char*>(std::cout, ", "));

    return 0;
}
