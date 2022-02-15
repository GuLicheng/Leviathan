#include <iostream>
#include <vector>
#include <string>
#include <string_view>

template <typename CharT, typename T>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const std::vector<T>& rhs)
{
    os << '[';
    auto begin = rhs.begin(), end = rhs.end();
    for (auto vec_iter = begin; vec_iter != end; ++vec_iter)
    {
        if (vec_iter != begin)
            os << ", ";
        os << *vec_iter;
    }
    return os << ']';
}

#include <lv_cpp/named_tuple.hpp>

using Names = std::vector<std::string_view>;
using info = named_tuple<
    field<"names", Names>,
    field<"help", std::string_view>,
    field<"default", std::string_view>,
    field<"args", int, []{ return 1; }>,
    field<"const", bool>,
    field<"required", bool>
>;

class argument_parser
{
public:

    argument_parser() = default;
    argument_parser(const argument_parser&) = delete;
    argument_parser& operator=(const argument_parser&) = delete;

    template <typename... Args>
    void add_argument(Args... args)
    {
        m_args.emplace_back(std::move(args)... );
    }

    void show() const 
    {
        for (auto& i : m_args)
            std::cout << i << '\n';
    }

private:

    std::vector<info> m_args;
};


int main()
{
    auto parser = argument_parser();
    parser.add_argument(arg<"help"> = "Test", arg<"names"> = Names{"--v", "-v"});
    parser.show();
}