#include <leviathan/io/console.hpp>

using namespace leviathan::io;

void test_for_output();

int main()
{
    // test_for_input();
    test_for_output();
}

void set_foreground_color(console_color color, const char* info)
{
    console::set_foreground_color(color);
    console::write(info);
}

void set_background_color(console_color color, const char* info)
{
    console::set_background_color(color);
    console::write(info);
}

void set_font_style(console_fontstyle fontstyle, const char* info)
{
    console::set_font(fontstyle);
    console::write(info);
}

void test_for_output()
{
    set_foreground_color(console_color::black, "This is black\n");
    set_foreground_color(console_color::blue, "This is blue\n");
    set_foreground_color(console_color::green, "This is green\n");
    set_foreground_color(console_color::cyan, "This is cyan\n");
    set_foreground_color(console_color::red, "This is red\n");
    set_foreground_color(console_color::white, "This is white\n");
    set_foreground_color(console_color::magenta, "This is magenta\n");
    set_foreground_color(console_color::yellow, "This is yellow\n");

    set_foreground_color(console_color::bright_black, "This is bright_black\n");
    set_foreground_color(console_color::bright_blue, "This is bright_blue\n");
    set_foreground_color(console_color::bright_green, "This is bright_green\n");
    set_foreground_color(console_color::bright_cyan, "This is bright_cyan\n");
    set_foreground_color(console_color::bright_red, "This is bright_red\n");
    set_foreground_color(console_color::bright_white, "This is bright_white\n");
    set_foreground_color(console_color::bright_magenta, "This is bright_magenta\n");
    set_foreground_color(console_color::bright_yellow, "This is bright_yellow\n");

    console::reset_color();

    set_background_color(console_color::black, "This is black\n");
    set_background_color(console_color::blue, "This is blue\n");
    set_background_color(console_color::green, "This is green\n");
    set_background_color(console_color::cyan, "This is cyan\n");
    set_background_color(console_color::red, "This is red\n");
    set_background_color(console_color::white, "This is white\n");
    set_background_color(console_color::magenta, "This is magenta\n");
    set_background_color(console_color::yellow, "This is yellow\n");

    set_background_color(console_color::bright_black, "This is bright_black\n");
    set_background_color(console_color::bright_blue, "This is bright_blue\n");
    set_background_color(console_color::bright_green, "This is bright_green\n");
    set_background_color(console_color::bright_cyan, "This is bright_cyan\n");
    set_background_color(console_color::bright_red, "This is bright_red\n");
    set_background_color(console_color::bright_white, "This is bright_white\n");
    set_background_color(console_color::bright_magenta, "This is bright_magenta\n");
    set_background_color(console_color::bright_yellow, "This is bright_yellow\n");

    console::reset_color();

    set_font_style(console_fontstyle::bold, "This is bold\n");
    set_font_style(console_fontstyle::italic, "This is italic\n");
    set_font_style(console_fontstyle::faint, "This is faint\n");
    set_font_style(console_fontstyle::underline, "This is underline\n");
    set_font_style(console_fontstyle::slow_blink, "This is slow_blink\n");
    set_font_style(console_fontstyle::rapid_blink, "This is rapid_blink\n");

    console::reset();
}

