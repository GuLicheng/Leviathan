#include <lv_cpp/io/console.hpp>
#include "../../utils/struct.hpp"
#include <vector>
#include <set>


using leviathan::io::console_color;
using leviathan::io::console_style;

std::vector vec{1, 2, 3, 4, 50};
std::set set{0, 0, 0, 1, 1};

namespace std
{
    template <typename T, typename U>
    std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& __pair)
    {
        return os << '{' << __pair.first << ',' << ' ' << __pair.second << '}';
    }
} // ...


void test_for_output();
void test_for_input();
void test_for_multi_paras();
void test_for_color_style();

int main()
{
    // test_for_input();
    test_for_output();
    test_for_multi_paras();
    test_for_color_style();
}

void test_for_input()
{
    auto line = console::read_line();
    console::write_line(line);
    int x;
    double y;
    char buffer[128];
    std::string str;
    console::read(x);
    console::read(y);
    console::read(str);
    console::read(buffer);
    console::write_line(x);
    console::write_line(y);
    console::write_line(buffer);
    console::write_line(str);
}

void test_for_output()
{
    console::set_foreground_color(console_color::yellow);
    console::write_line(7);
    console::write_line(false);
    console::write_line<int>(true);
    console::set_background_color(console_color::blue);
    console::write_line(3.14);
    console::set_background_color(console_color::black);
    console::write_line("hello");
    console::write_line("hello world", 8);
    console::set_fontstyle(console_style::italic);
    console::write_line(std::string("italic"));
    console::write_line("hello world!", 6, 5);
    console::write_line_type<std::string>();
    console::text_writer();
    console::text_reader();
    console::set_fontstyle(console_style::reset);
    console::write_line("reset");
    console::write_line(vec);
    console::write_line(set);
    console::text_error();
    console::reset_color();
    console::write_line(std::make_tuple(1, "hello world", false));
    console::write_line(std::make_pair(1, 2));
    console::reset();
}

void test_for_multi_paras()
{
    // console::write_line_multi
    //     (empty_class{}, 
    //     1, 
    //     foo(), 
    //     ::std::make_pair(1, 3), 
    //     "hello world", 
    //     ::isspace, 
    //     true,
    //     std::make_tuple(0));
    console::set_foreground_color(console_color::blue);
    console::write_line("Multi-paras pack: ({0}, {1}, {2}, {3}, {4}, {5}, {6}, {7})",
        empty_class{}, 
        1, 
        foo(), 
        ::std::make_pair(1, 3), 
        "hello world", 
        ::isspace, 
        true,
        std::make_tuple(0));
    console::write_line_type<int, int&&, double, bool, std::string>();
}

void test_for_color_style()
{
    console::set_foreground_color(console_color::yellow);
    console::write_line("yellow");

    console::set_foreground_color(console_color::bright_yellow);
    console::write_line("bright yellow");

    console::set_foreground_color(console_color::bright_white);
    console::write_line("bright white");

    console::set_fontstyle(console_style::slow_blink);
    console::write_line("slow blink");

    console::set_fontstyle(console_style::italic);
    console::write_line("italic");

    console::reset();
}

