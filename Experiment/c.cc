
#include <iostream>
#include <vector>
#include <lv_cpp/named_tuple.hpp>

struct info
{
    std::vector<std::string_view> m_arg_names;

    named_tuple<
        field<"help", std::string_view>,
        field<"default", std::string_view>,
        field<"nargs", std::string_view, []
              { return 1; }>,
        field<"const", std::string_view>,
        field<"require", std::string_view>>
        m_other;
    friend std::ostream &operator<<(std::ostream &os, const info &i)
    {
        os.setf(std::ios_base::boolalpha);
        os << "Names = [";
        for (size_t k = 0; k < i.m_arg_names.size(); ++k)
        {
            if (k != 0)
                os << ", ";
            os << i.m_arg_names[k];
        }
        os
            << "] help = " << i.m_other["help"_arg] << " nargc = " << i.m_other["nargs"_arg] << " const ?: " << i.m_other["const"_arg]
            << " required ?: " << i.m_other["require"_arg] << " value = " << i.m_other["default"_arg];
        os.unsetf(std::ios_base::boolalpha);
        return os;
    }
};

int main()
{
    info i;
    std::cout << i << '\n';
}