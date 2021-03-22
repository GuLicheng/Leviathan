/*
    1. https://en.wikipedia.org/wiki/ANSI_escape_code
    
    2. basic_console can work for range, string, pair, tuple 
    If you have overloaded basic_ostream for your own class, 
    basic_console will perform as what your have overloaded
*/
#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#include <lv_cpp/io/enum.hpp>
#include <lv_cpp/io/constraint.hpp>
#include <lv_cpp/meta/template_info.hpp>

#include <tuple>


namespace leviathan::io
{



    template <typename _Char> 
    struct basic_console_base;

    template <>
    struct basic_console_base<char>
    {
        inline static auto& os = ::std::cout;
        inline static auto& is = ::std::cin;
        inline static auto& err = ::std::cerr;
    };

    template <>
    struct basic_console_base<wchar_t>
    {
        inline static auto& os = ::std::wcout;
        inline static auto& is = ::std::wcin;
        inline static auto& err = ::std::wcerr;
    };

    template <typename _Char> 
    class basic_console : private basic_console_base<_Char>
    {
        using basic_console_base<_Char>::os;
        using basic_console_base<_Char>::is;
        using basic_console_base<_Char>::err;
        using base = ::std::basic_ostream<_Char>;

        // static class 
        basic_console() = delete;
                

        template <typename... Ts, size_t... Idx>
        static void print_tuple_impl(const ::std::tuple<Ts...>& __tuple, 
            ::std::index_sequence<Idx...>, const _Char* left, const _Char* right, const _Char* delim)
        {
            write(left);
            ((Idx == 0 ? 
                write(::std::get<0>(__tuple)) : 
                (write(delim), write(::std::get<Idx>(__tuple)))), ...);
            write(right);
        }

        template <int N, int Max>
        struct index_selector
        {
            template <typename... Ts>
            static void apply(const ::std::tuple<Ts...> &t, int index)
            {
                if constexpr (N == Max)
                {
                    return ;
                }
                else if (index == N)
                {
                    // return std::to_string(std::get<N>(t));
                    basic_console<_Char>::write(::std::get<N>(t));
                }
                else
                {
                    // return index_selector<N + 1, Max>::apply(t, index);
                    index_selector<N + 1, Max>::apply(t, index);
                }
            }
        };

    public:
        using char_type = typename base::char_type;
        using int_type = typename base::int_type;
        using pos_type = typename base::pos_type;
        using off_type = typename base::off_type;
        using traits_type = typename base::traits_type;

        template <typename... Ts>
        static void dynamic_get(const ::std::tuple<Ts...> &t, int index)
        {
            index_selector<0, sizeof...(Ts)>::apply(t, index);
        }

        template <typename... Ts>
        static void format(const ::std::basic_string<char_type>& fmt, Ts&&... ts)
        {
            const auto t = ::std::forward_as_tuple(ts...);
            for (size_t i = 0; i < fmt.size(); ++i)
            {
                if (fmt[i] != '{') write(fmt[i]);
                else
                {
                    int j = i + 1;
                    while (fmt[j] != '}') ++j;
                    int index = ::std::stoi(fmt.substr(i + 1, j - i - 1));
                    dynamic_get(t, index);
                    i = j;
                }
            }
    
        }


        static void write(bool val)
        {
            write(val ? "true" : "false");
        }

        static void write_line(bool val)
        {
            write(val);
            write_line();
        }

        template <typename T> requires (printable<T, _Char>)
        static void write(const T& val)
        {
            os << val; 
        }

        template <typename T> requires (printable<T, _Char>)
        static void write_line(const T& val)
        {
            write(val);
            write_line();
        }


        static void write_line()
        { ::std::endl(os); }

        // The iterator must be copyable
        template <typename T> requires (!printable<T, _Char> && range_c<T>)
        static void write(const T& rg)
        {
            write('[');
            auto first = ::std::ranges::begin(rg);
            auto last = ::std::ranges::end(rg);
            int i = 0;
            for (auto iter = first; iter != last; ++iter)
            {
                if (i++) write(", ");
                write(*iter);
            }
            write(']');
        }

        template <typename T> requires (!printable<T, _Char> && range_c<T>)
        static void write_line(const T& rg)
        {
            write(rg);
            write_line();
        }

        template <typename T1, typename T2> requires (!printable<::std::pair<T1, T2>, _Char>)
        static void write(const ::std::pair<T1, T2>& __pair)
        {
            write('(');
            write(__pair.first);
            write(", ");
            write(__pair.second);
            write(')');
        }

        template <typename T1, typename T2> requires (!printable<::std::pair<T1, T2>, _Char>)
        static void write_line(const ::std::pair<T1, T2>& __pair)
        {
            write(__pair);
            write_line();
        }

        template <typename... Ts> requires (!printable<::std::tuple<Ts...>, _Char>)
        static void write(const ::std::tuple<Ts...>& __tuple)
        {
            print_tuple_impl(__tuple, ::std::make_index_sequence<sizeof...(Ts)>(), "(", ")", ", ");
        }

        template <typename... Ts> requires (!printable<::std::tuple<Ts...>, _Char>)
        static void write_line(const ::std::tuple<Ts...>& __tuple)
        {
            write(__tuple);
            write_line();
        }

        template <typename T> requires (!printable<T, _Char> && !range_c<T> && !string_c<T>)
        static void write(const T&)
        { write_type<T>(); }

        template <typename T> requires (!printable<T, _Char> && !range_c<T> && !string_c<T>)
        static void write_line(const T& val)
        {
            write(val);
            write_line();
        }

        // at least one parameter
        template <typename T1, typename... Ts>
        static void write_multi(const T1& t1, const Ts&... ts)
        {
            write(t1);
            ((write(' '), write(ts)), ...);
        }

        // at least one parameter
        template <typename T1, typename... Ts>
        static void write_line_multi(const T1& t1, const Ts&... ts)
        {
            write_multi(t1, ts...);
            write_line();
        }

        // at least one parameter
        template <typename T1, typename... Ts>
        static void write_lines_multi(const T1& t1, const Ts&... ts)
        {
            write_line(t1);
            (write_line(ts), ...);
        }

        // for type
        template <typename... Ts>
        static void write_type()
        {
            write_multi(type_to_str<Ts>()...);
        }

        template <typename... Ts>
        static void write_line_type()
        { 
            write_type<Ts...>();
            write_line();
        }

        template <typename... Ts>
        static void write_lines_type()
        { 
            (write_line_type<Ts>(), ...);
        }

        // for instance please use macro PrintTypeCategory
        // Since we cannot make sure whether (assume declearing: int a = 0;) 
        // object a is int or int&&



        // Here are some functions for screen/console
        static void clear()
        { write("\033[2J"); }

        static void set_foreground_color(console_color c)
        { write(color[int(c)]); }

        static void set_background_color(console_color c)
        { write(color[int(c) + 16]); }

        static void set_fontstyle(console_style s)
        { write(style[int(s)]); }

        static void reset_color()
        { write("\033[49m"); }

        static void reset()
        { set_fontstyle(console_style::reset); }


        // method for input
        template <typename _String = ::std::basic_string<_Char>>
        static _String read_line()
        {
            _String line;
            ::std::getline(is, line);
            return line;
        }

        template <typename _String = ::std::basic_string<_Char>>
        static _String read_line(_Char delim)
        {
            _String line;
            ::std::getline(is, line, delim);
            return line;
        }

        template <typename T> requires number_c<T>
        static void read(T& val)
        { is >> val; }

        static void read(_Char* str)
        { is >> str; }

        template <typename _String> requires string_c<_String>
        static void read(_String& str)
        { is >> str; }

        // attribute
        static auto& text_writer()
        { return os; }

        static auto& text_reader()
        { return is; }

        static auto& text_error()
        { return err; }

    }; //  end of class 


} //  namespace leviathan

using console = leviathan::io::basic_console<char>;
using wconsole = leviathan::io::basic_console<wchar_t>;


#endif