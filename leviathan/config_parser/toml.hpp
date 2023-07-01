/*
    Reference:
        - https://mojotv.cn/2018/12/26/what-is-toml
        - https://toml.io/cn/v1.0.0
        - https://github.com/toml-lang/toml/blob/1.0.0/toml.abnf
*/

#pragma once

#include <leviathan/config_parser/common.hpp>
#include <leviathan/config_parser/value.hpp>
#include <leviathan/string/string_extend.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <variant>
#include <ranges>
#include <chrono>
#include <format>

namespace leviathan::config::toml
{
    using leviathan::string::ltrim;
    using leviathan::string::rtrim;
    using leviathan::string::trim;
    using leviathan::string::arithmetic;
    using leviathan::string::string_hash_key_equal;
    // using leviathan::string::whitespace_delimiters;
    using leviathan::string::string_viewable;

    using std::string_view;
    using std::string;

    // Some exceptions
    struct toml_parse_error : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    struct bad_toml_value_access : std::exception
    {
        const char* what() const noexcept override
        { return "bad_toml_value_access"; }
    };

    template <typename... Args>
    void throw_toml_parse_error(string_view fmt, Args&&... args)
    {
        auto msg = std::vformat(fmt, std::make_format_args((Args&&) args...));
        throw toml_parse_error(msg);
    }

    // Can we use std::chrono directly?
    // YYYY-MM-DDTHH:MM:SS.XXXX+HH:MM
    struct toml_data_time
    {
        int m_year = 0;
        int m_month = 0;
        int m_day = 0;

        int m_hour = 0;
        int m_minute = 0;
        int m_second = 0;
        int m_microsecond = 0;

        int m_hour_offset = 0;
        int m_minute_offset = 0;

    };

    template <typename T>
    struct use_pointer 
    {
        constexpr static bool value = sizeof(T) > 16;

        using type = std::conditional_t<value, std::unique_ptr<T>, T>;
    };

    struct to_shared_ptr
    {
        template <typename T>
        constexpr auto operator()(T&& t) const
        {
            using U = std::remove_cvref_t<T>;
            if constexpr (use_pointer<U>::value)
            {
                return std::make_shared<U>((T&&)t);
            }
            else
            {
                return t;
            }
        }
    };

    class toml_value;

    using toml_boolean = bool;
    using toml_integer = int64_t;
    using toml_float = double;
    using toml_string = string;
    using toml_array = std::vector<toml_value>;
    using toml_table = std::unordered_map<toml_string, toml_value, string_hash_key_equal, string_hash_key_equal>;

    using toml_value_base = value_base<
        std::variant, 
        to_shared_ptr,
        toml_boolean,
        toml_integer,
        toml_float,
        toml_string,
        toml_array, 
        toml_table,
        toml_data_time
    >;

    class toml_value : public toml_value_base
    {
    public:

        toml_value() = delete;

        using toml_value_base::toml_value_base;
        using toml_value_base::operator=;

    };

    namespace detail
    {
        struct config
        {
            constexpr static int value = 256;

            int operator()(size_t x) const
            {
                [[assume(x < value)]];
                static string_view sv = R"(_-. )";
                return isalnum(x) || sv.contains(x);
            }
        };

        std::vector<string> split_keys(string_view key)
        {
            assert(key.size());

            std::vector<string> result;

            static auto table = make_character_table(config());

            auto parse_quote = [&](char ch) {

                auto sv = key;

                assert(sv.size() && sv.front() == ch);

                sv.remove_prefix(1);

                auto next_quote = sv.find(ch);

                if (next_quote == sv.npos)
                {
                    throw_toml_parse_error("Parsing quote key error.");
                }

                result.emplace_back(sv.substr(0, next_quote));

                sv.remove_prefix(1 + next_quote);

                key = ltrim(sv);
            };

            while (1)
            {
                char ch = key.front();

                if (ch == '"')
                {
                    parse_quote('"');
                }
                else if (ch == '\'')
                {
                    parse_quote('\'');
                }
                else if (table[ch])
                {
                    // pase bar string
                    auto head = key.begin(), tail = key.end();
                    for (; head != tail; ++head)
                    {
                        auto ch2 = *head;
                        if (ch2 == '.' || ch2 == ' ')
                        {
                            result.emplace_back(key.begin(), head);
                            key = ltrim(string_view(head, tail));
                            break;
                        }
                        else if (!table[*head])
                        {
                            throw_toml_parse_error("Invalid character when parsing bare key."); 
                        }
                    }
                }
                else
                {
                    throw_toml_parse_error("Invalid character when parsing bare key.");
                }

                if (key.empty())
                {
                    throw_toml_parse_error("Only key.");
                }
            
                ch = key.front();

                if (ch == '=')
                {
                    // OK
                    return result;
                }
                else if (ch == '.')
                {
                    key.remove_prefix(1);
                    key = ltrim(key);
                    continue;
                }
                else
                {
                    throw_toml_parse_error("Expected '=' or '.', but got {}", ch);
                }
            }
        }

    }

    class parser
    {
        string m_context;
        std::vector<std::string_view> m_lines;
        string_view m_line; // current line for parsing.
        toml_table m_global;

        static constexpr string_view linefeed = "\n";

    public:

        parser(string context) 
            : m_context(std::move(context)) { }
    
        toml_value operator()() &&
        {
            read_lines();
            for (auto line : m_lines)
            {
                // std::cout << line << '\n';
                // std::cout << std::format("line = ({}) and last char is ({})\n", line, line.back());
            }
            parse();
            // return std::make_shared<toml_table>(std::move(m_global));
            return std::move(m_global);
        }

    private:

        /**
         * @brief Split context by line and remove empty line.
        */
        void read_lines()
        {
            // Every line may ends with \r\n on Windows.
            // We replace all \r\n to \n, this is not efficient but simple.
            m_context = leviathan::string::replace(std::move(m_context), "\r\n", "\n");
            for (auto line : m_context | std::views::split(linefeed))
            {
                m_lines.emplace_back(line);
            }
        }

        /**
         * @brief Return next non-empty line
         * 
         * @return Empty line if parse over, otherwise next line context after ltrim.
        */
        string_view getline()
        {

        }

        void parse()
        {
            for (auto line : m_lines)
            {
                m_line = ltrim(line);

                // Empty line or just commit.
                if (m_line.empty() || m_line.front() == '#')
                {   
                    // U+0000 - U+0008 and U+000A - U+001F,U+007F should not appear in commit.
                    continue;
                }

                if (m_line.front() == '[')
                {
                    // [table] or [[table_array]]
                    parse_table_or_table_array();
                }
                else
                {
                    // parse key and value.
                    parse_entry();
                }
            }
        }

        void parse_entry()
        {
            // parse key
            parse_key();

            // match '='
            m_line = ltrim(m_line);

            if (m_line.empty() || m_line.front() != '=')
            {
                throw_toml_parse_error("");
            }

            // parse value
            parse_value();
        }

        void parse_value();

        void parse_table_or_table_array()
        {
            assert(m_line.front() == '[');

            advance_unchecked(1); // eat [

            if (current() == '[')
            {
                parse_table_array();
            }
            else
            {
                parse_table();
            }
        }

        void parse_table()
        {

        }

        static std::vector<string> split_keys(string_view key)
        {
            struct config
            {
                int operator()(size_t x) const
                {
                    [[assume(x < 256)]];
                    static string_view sv = R"(_-. '")";
                    return isalnum(x) || sv.contains(x);
                }
            };

            static auto table = make_character_table(config());

            for (size_t i = 0; i < key.size(); ++i)
            {
                
            }

        }

        void parse_table_array()
        {
            advance_unchecked(1); // eat '['

            auto head = 0;
            auto tail = m_line.find_first_of("]]");
            if (tail == m_line.npos)
            {
                throw_toml_parse_error("");
            }

            auto table_name = m_line.substr(0, tail);

        }

        void parse_key()
        {

            struct raw_character_config
            {
                int operator()(size_t x) const
                {
                    return isalnum(x) || x == '_' || x == '-';
                }
            };
            
            static auto raw_characters = make_character_table(raw_character_config());

            std::vector<std::string> keys;

            if (current() == '"')
            {
                // quoted keys
                advance_unchecked(1);
                
            }
            else if (current() == '\'')
            {

            }
            else
            {
                // bare key
                while (1)
                {
                    auto head = m_line.begin();
                    auto tail = std::find_if(m_line.begin(), m_line.end(), [](auto ch) {
                        return raw_characters[ch];
                    });
                }



            }
        }

        // void match_and_advance(char ch)
        // {
        //     if (m_cur.empty())
        //     {
        //         return false;
        //     }
        //     bool result = current() == ch;
        //     advance_unchecked(1);
        //     return result;
        // }

        void skip_whitespace()
        {
            struct whitespace_config
            {
                constexpr int operator()(size_t i) const
                {
                    [[assume(i < 128)]];
                    constexpr std::string_view sv = " \r\n\t";
                    return sv.contains(i);
                }
            };

            static auto whitespaces = make_character_table(whitespace_config());

            auto is_whitespace = [](char ch) { return whitespaces[ch]; };
            for (; m_line.size() && is_whitespace(current()); advance_unchecked(1));
        }

        char peek(int n) const
        { return m_line[n]; }

        char current() const
        { return peek(0); }

        void advance_unchecked(size_t n)
        { m_line.remove_prefix(n); }

    };


} // namespace leviathan::config::toml
















