// https://docs.python.org/zh-cn/3/library/argparse.html#the-add-argument-method

/*
    Py:
        add_argument():
        name or flag
        action X
        nargs
        const
        default
        type X
        choices X
        required
        help X
        metavar
        dest X
*/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string_view>
#include <regex>
#include <algorithm>
#include <assert.h>

enum consume : int
{
    at_least_one,      // '+'
    variadic,          // '*'
    one_if_possible,   // '?'
    sentinel
};

enum
{
    flag_arg,
    pos_arg
};

class argument_parser;

struct argument
{
    std::string m_name;
    std::string m_default = "";
    std::string m_help = "";
    std::string m_metavar = "";
    int m_nargs = 1;
    bool m_required = false;
    std::vector<std::string> m_values;  // value : one or more
};

class argument_builder
{
public:

    // offer some api for build 
    using self_type = argument_builder;

    self_type default_value(std::string_view value)&&
    {
        m_arg->m_default = value;
        return *this;
    }

    self_type required()&&
    {
        m_arg->m_required = true;
        return *this;
    }

    self_type help(std::string_view info)&&
    {
        m_arg->m_help = info;
        return *this;
    }

    self_type nargs(consume num)&&
    {
        m_arg->m_nargs = num;
        return *this;
    }

private:

    friend class argument_parser;

    argument_builder(argument* arg) : m_arg{ arg } { }
    argument_builder(const argument_builder&) = default;

    argument* m_arg;
};

class argument_parser
{
public:
    argument_parser() = default;
    argument_parser(const argument_parser&) = delete;
    argument_parser& operator=(const argument_parser&) = delete;


    argument_builder add_argument(std::string_view name)
    {
        auto ref = (name.starts_with('-') ? m_flag_args : m_pos_args).emplace_back(argument{
            .m_name = std::string(name)
            });
        std::cout << "[" << ref.m_name << "]\n";
        return { std::addressof(ref) };
    }

    void parse_argument(int argc, char const* argv[])
    {
        std::cout << argv[0] << '\n';
        for (int i = 1; i < argc; i = parse(i, argc, argv));
    }

private:

    int parse(int cur, int argc, char const* argv[])
    {
        // m_pos_args

        int len = ::strlen(argv[cur]);
        if (len == 0)
            throw std::runtime_error("Empty Args.");

        bool is_position;

        // Argument should be such as "--version", "-v", or "only_alpha" 
        if (argv[cur][0] == '-')
            is_position = false;
        else if (::isalpha(argv[cur][0]))
        {
            if (m_pos >= m_pos_args.size())
                throw std::runtime_error("Too must position arguments.");
            is_position = true;
        }
        else
            throw std::runtime_error("Error Token.");

        return try_consume_argument(is_position, cur + 1, argc, argv);
    }

    static int find_entry(const std::vector<argument>& args, std::string_view name)
    {
        //return std::distance(std::ranges::find(args, name, &argument::m_name), args.end());
        for (int i = 0; i < args.size(); ++i)
            if (args[i].m_name == name)
                return i;
        return args.size();
    }

    int try_consume_argument(bool is_pos, int cur, int argc, char const* argv[])
    {
        assert(cur < argc);
        auto& ref = is_pos ? m_pos_args : m_flag_args;
        int idx = is_pos ? m_pos : find_entry(ref, argv[cur - 1]);
        
        if (idx >= ref.size())
            throw std::runtime_error("Unknown Name [" + std::string(argv[cur - 1]) + "]");
        int num = ref.at(idx).m_nargs;

        if (num > 0)
        {
            if (num + cur > argc)
                throw std::runtime_error("Not Enough Argument.");

            for (int i = 0; i < num; ++i, ++cur)
            {
                if (is_name(argv[cur]))
                    throw std::runtime_error("Got Name or Flag.");
                std::cout << "Idx = " << idx << '\n';
                ref.at(idx).m_values.emplace_back(argv[cur]);
            }
        }
        else
        {
            if (num == static_cast<int>(consume::at_least_one))
            {
                if (is_name(argv[cur]))
                    throw std::runtime_error("At Least One Argument.");
                for (; cur < argc && !is_name(argv[cur]); ++cur)
                    ref.at(idx).m_values.emplace_back(argv[cur]);
            }
            else if (num == static_cast<int>(consume::one_if_possible))
            {
                if (!is_name(argv[cur]))
                    ref.at(idx).m_values.emplace_back(argv[cur++]);
            }
            else //  num == consume::variadic
            {
                for (; cur < argc && !is_name(argv[cur]); ++cur)
                    ref.at(idx).m_values.emplace_back(argv[cur]);
            }
        }
        return cur;
    }


    bool is_name(std::string_view s)
    {
        return std::ranges::find(m_flag_args, s, &argument::m_name) != m_flag_args.end()
            || std::ranges::find(m_pos_args, s, &argument::m_name) != m_pos_args.end();
    }

    std::vector<argument> m_flag_args;
    std::vector<argument> m_pos_args;
    int m_pos = 0;
};

int main(int argc, char const* argv[])
{

    argument_parser parser;

    parser.add_argument("--name");
    parser.add_argument("--age");

    char const* argvs[] = {
        "xxx.exe", "--name", "1", "--age", "2"
    };

    argc = 5;

    try
    {
        parser.parse_argument(argc, argvs);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    std::cout << "Ok\n";

    return 0;
}
