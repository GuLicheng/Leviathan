/*
    TODO: 
        nargs
*/
#include <lv_cpp/meta/type_list.hpp>
#include <lv_cpp/meta/template_info.hpp>
#include <lv_cpp/string/opt.hpp>
#include <lv_cpp/tuples/core.hpp>
#include <lv_cpp/named_tuple.hpp>

#include <optional>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_set>
#include <string_view>
#include <regex>

#include <ctype.h>

template <typename... Ts>
void println(Ts... x)
{
    (std::cout << ... << x) << '\n';
}

using leviathan::cat_string;

//////////////////////////////////////////////////////
// Some Helper
//////////////////////////////////////////////////////
template <typename Target>
struct argument_cast_t;  // declration

template <typename CharT, typename Traits, typename Alloc>
struct argument_cast_t<std::basic_string<CharT, Traits, Alloc>>
{
    template <typename I>
    std::basic_string<CharT, Traits, Alloc> operator()(I first, I last) const
    {
        return std::basic_string<CharT, Traits, Alloc>{first, last};
    }

    template <typename R>
    std::basic_string<CharT, Traits, Alloc> operator()(R&& rg) const
    {
        return (*this)(std::begin(rg), std::end(rg));
    }
};

template <std::signed_integral SignedInteger>
struct argument_cast_t<SignedInteger>
{
    template <typename R>
    SignedInteger operator()(R&& rg) const
    {
        return static_cast<SignedInteger>(std::stoll(rg));
    }
};

template <std::unsigned_integral UnsignedInteger>
struct argument_cast_t<UnsignedInteger>
{
    template <typename R>
    UnsignedInteger operator()(R&& rg) const
    {
        return static_cast<UnsignedInteger>(std::stoull(rg));
    }
};

template <std::floating_point FloatingPoint>
struct argument_cast_t<FloatingPoint>
{
    template <typename R>
    FloatingPoint operator()(R&& rg) const
    {
        // return static_cast<FloatingPoint>(std::stold(rg)); long double ?
        return static_cast<FloatingPoint>(std::stod(rg));
    }
};


template <typename T, typename U>
std::optional<T> argument_cast(U&& u)
{
    try
    {
        return argument_cast_t<T>()((U&&) u);
    }
    catch(...)
    {
        return { };
    }
}

struct check
{

    static bool check_optional(std::string_view sv)
    { return sv.starts_with('-'); }
    
    static bool check_name(const std::vector<std::string_view>& names)
    {
        if (names.size() > 1)
            return std::ranges::all_of(names, &check::check_optional);
        return true;
    }

};


struct parser_result
{
    leviathan::string_hashmap<std::unordered_map, size_t> m_maps;
    std::vector<std::string> m_values; // store value

    void display() const
    {
        for (auto& [k, v] : m_maps)
            println("k = ", k, " and v = ", m_values[v]);
    }

    template <typename T>
    std::optional<T> get(std::string_view name)
    {
        auto iter = m_maps.find(name);
        if (iter == m_maps.end())
        {
            return { };
        }
        return argument_cast<T>(m_values[iter->second]);
    }

};
class argument_parser
{
public:
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

    argument_parser() = default;
    argument_parser(const argument_parser&) = delete;
    argument_parser& operator=(const argument_parser&) = delete;

    template <typename... Args>
    void add_argument(Args... args)
    {
        using leviathan::meta::find_first_index_of;
        using TypeList = std::tuple<Args...>;

        constexpr auto npos = static_cast<size_t>(-1);

        info i;
        auto params = std::forward_as_tuple(std::move(args)...);

        auto register_name = [this]<typename T>(const T& arg)
        {
            // check
            if constexpr (std::is_same_v<T, arg_names>)
            {
                for (auto name : arg.Value)
                    if (this->m_names.count(name))
                        // this->m_errlog.emplace_back(str("Name") + str(name) + "Already Existed");
                        this->m_errlog.emplace_back(cat_string("Name ", name, " Already Existed"));
                    else this->m_names.emplace(name);
            }
        };

        (register_name(args), ...);


        // check const
        if (i.m_is_const && i.m_default_value.empty())
            // this->m_errlog.emplace_back(str("Const Attribute") + str(i.m_arg_names[0]) + "Must Have Default Value");
            this->m_errlog.emplace_back(cat_string("Const Attribute ", i.m_arg_names[0], " Must Have Default Value"));

        // check name, if short name supported, the long name must started with `--`, such as --version, -v
        // if short name not supported, longname could be `version` or `--version`
        if (!check::check_name(i.m_arg_names))
            this->m_errlog.emplace_back("Error names");


        // check_require

        m_args.emplace_back(std::move(i));
    }

    std::pair<parser_result, bool> parse_args(int argc, char const *argv[]) 
    {
        m_prop_name = argv[0];

        std::vector<size_t> indices;

        for (size_t i = 0; i < m_args.size(); ++i)
        {
            if (std::ranges::none_of(m_args[i].m_arg_names, &check::check_optional))
            {
                indices.emplace_back(i);
            }
        }

        size_t idx = 0;

        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];
            // println("argv = (", arg, ") with Length:", arg.size());
            std::match_results<std::string_view::iterator> base_match;
            if (std::regex_match(arg.begin(), arg.end(), base_match, Pattern))
            {
                if (base_match.size() != 3)
                    this->m_errlog.emplace_back("Error argv");
                std::string_view value1 = { base_match[1].first, base_match[1].second };
                std::string_view value2 = { base_match[2].first, base_match[2].second };
                auto iter = std::ranges::find_if(m_args, [=](const info& i) {
                    return std::ranges::find(i.m_arg_names, value1) != i.m_arg_names.end();
                });
                
                if (iter == m_args.end())
                    this->m_errlog.emplace_back(cat_string("Unknown Argument ", argv[i])); 
                if (iter->m_is_const)
                    this->m_errlog.emplace_back(cat_string("Const Argument ", iter->m_arg_names[0], " Cannot be Modified"));
                iter->m_default_value = value2;
                // println("key = ", key, " value1 = ", value1, " value2 = ", value2);
            }
            else
            {
                if (idx >= indices.size())
                    this->m_errlog.emplace_back("Too much arguments");
                m_args[indices[idx]].m_default_value = arg;
            }

        }
    
        parser_result ret;

        auto lretrive = [](std::string_view sv) {
            auto idx = sv.find_first_not_of('-');
            return sv.substr(idx);
        };

        for (auto& info : m_args)
        {
            
            if (info.m_required && info.m_default_value.empty())
                this->m_errlog.emplace_back(cat_string("Required Argument ", info.m_arg_names[0], " is Null"));
            ret.m_values.emplace_back(info.m_default_value);
            for (auto sv : info.m_arg_names)
                ret.m_maps[std::string(lretrive(sv))] = ret.m_values.size() - 1;
        }


        return { ret, m_errlog.empty() };
    }

    template <typename T>
    std::optional<T> get(const std::string& name)
    {
        auto iter = std::find_if(m_args.begin(), m_args.end(), [&](const info& i)
        {
            return std::ranges::find(i.m_arg_names, name) != i.m_arg_names.end();
        });
        if (iter == m_args.end())
        {
            return { };
        }
        return argument_cast<T>(iter->m_default_value);
    }

    void report_error() const 
    {
        for (auto& msg : m_errlog)
            std::cout << msg << '\n';
    }

private:

    inline static const std::regex Pattern{ "(.*)=(.*)" };

    std::vector<info> m_args;
    std::string m_prop_name;
    leviathan::string_hashset<std::unordered_set> m_names;
    std::vector<std::string> m_errlog; 
};

int main()
{
    info i;
    std::cout << i << '\n';
}