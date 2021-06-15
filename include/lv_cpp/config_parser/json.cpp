#include "./json.hpp"

namespace leviathan::parser
{

    Lexer::Lexer(std::string context) 
        : m_context{std::move(context)}, m_cur{}, m_errors{}, m_parse_result{}
    {
        // console::write_line("the length of parse string is {0}", m_context.size());
    }

    Lexer& Lexer::parse()
    {
        while (!is_over())
        {
            console::write_line("m_cur is {0} and char is {1}", m_cur, m_context[m_cur]);
            switch (m_context[m_cur])
            {
                case ' ':
                case '\t': 
                case '\r':
                case '\n': skip_blank(); break;
                case 't': parse_true(); break;
                case 'f': parse_false(); break;
                case 'n': parse_null(); break;
                case '\'':
                case '"': parse_string(); break;
                case '-':
                case '+':
                case '.':
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': parse_number(); break;
                case '[': parse_array(); break;
                case '{': parse_object(); break;
                default:
                    std::string error_info = std::string("Got illegal character: ") + m_context[m_cur];
                    m_errors.emplace_back(0, "", std::move(error_info));
                    m_cur ++;
                    break;
            }   
        }
        return *this;
        
    }


    void Lexer::parse_string() { }

    int Lexer::next_blank() const
    {
        auto end = m_cur;
        const auto size = m_context.size();
        while (end < size)
        {
            char c = m_context[end];
            if (c == '\n' || c == ' ' || c == '\t' || c == '\r') break;
            else end ++;
        }
        return end;
    }

    void Lexer::parse_number()
    {
        auto begin = m_cur;
        while (!is_over())
        {
            char c = m_context[m_cur];
            if (c == '\n' || c == ' ' || c == '\t' || c == '\r') break;
            else m_cur++;
        }
        std::string number = m_context.substr(begin, m_cur - begin);
        try
        {
            double res = std::stod(number);
            m_parse_result.emplace_back(std::to_string(res));
        }
        catch(...)
        {
            std::string error_info = std::string("illegal number ") + number;
            m_errors.emplace_back(0, "", std::move(error_info));
        }
    }


    
    void Lexer::parse_array()
    {
        
    }

    void Lexer::parse_object()
    {
        
    }

    void Lexer::next()
    {
        if (!is_over())
            m_cur ++;
    }

    bool Lexer::is_over() const noexcept
    {
        return m_cur >= m_context.size();
    }

    bool Lexer::match(size_t len, const char* value)
    {
        if (m_context.size() - m_cur < len)
        {
            m_errors.emplace_back(0, "", "illegal length.");
            m_cur = next_blank();
            return false;
        }
        // bool res = std::string_view(&m_context[m_cur], len) == value;
        bool res = std::equal(&m_context[m_cur], &m_context[m_cur] + len, value);
        if (res) 
            m_parse_result.emplace_back(value);
        else
        {
            std::string error_info = std::string("Expected ") + value;
            m_errors.emplace_back(0, "", error_info);
        }
        m_cur += len;
        return res;
    }

    void Lexer::parse_null()
    {
        auto res = match(4, "null");
    }
    
    void Lexer::parse_true()
    {
        auto res = match(4, "true");
    }

    void Lexer::parse_false()
    {
        auto res = match(5, "false");
    }

    void Lexer::skip_blank()
    {
        m_cur ++;
        while (!is_over())
        {
            char c = m_context[m_cur];
            if (c == '\t' || c == '\n' || c == ' ' || c == '\r') 
                m_cur ++;
            else 
                break;
        }
    }


}