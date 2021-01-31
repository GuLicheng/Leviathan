#include <lv_cpp/io/console.hpp>

#include <vector>
#include <set>

using leviathan::io::console;
using leviathan::io::console_color;
using leviathan::io::console_style;

std::vector vec{1, 2, 3, 4, 50};
std::set set{0, 0, 0, 1, 1};

int main()
{
    console::set_foreground_color(console_color::yellow);
    console::write_line(7);
    console::write_line(false);
    console::write_line<bool>(true);
    console::set_background_color(console_color::blue);
    console::write_line(3.14);
    console::set_background_color(console_color::black);
    console::write_line("hello");
    console::write_line("hello world", 8);
    console::set_fontstyle(console_style::italic);
    console::write_line(std::string("italic"));
    console::write_line("hello world!", 6, 5);
    console::write_line_type<std::string>();
    console::write_line_type(std::string{});
    console::text_writer();
    console::text_reader();
    console::set_fontstyle(console_style::hight_light);
    console::write_line("hight light");
    console::set_fontstyle(console_style::reset);
    console::write_line("reset");
    console::write_line(vec);
    console::write_line(set);
    console::text_error();
    console::reset_color();
    console::reset();
}