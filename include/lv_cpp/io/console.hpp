#pragma once

#include "style.hpp"

#include <stdio.h>

namespace leviathan::io
{
    struct console
    {
        static void write(const char* ctrl_code)
        {
            ::printf(ctrl_code);
        }

        static void set_foreground_color(console_color color)
        {
            write(colors[color]);
        }

        static void set_background_color(console_color color)
        {
            write(colors[color + 8]); 
        }

        static void clear()
        { 
            write("\033[2J"); 
        }

        static void reset_color()
        { 
            write("\033[49m"); 
        }

        static void set_font(console_font font)
        {
            write(fonts[font]);
        }

        static void reset_font()
        {
            set_font(console_font::reset);
        }
    };
} // namespace leviathan::io

