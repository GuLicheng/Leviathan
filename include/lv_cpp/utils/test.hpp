#ifndef __TEST_HPP__
#define __TEST_HPP__

#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <lv_cpp/io/console.hpp>

namespace leviathan::test
{
    using namespace leviathan::io;
    class tester
    {
    public:
        tester(bool cond, int line, std::string correct, std::string expr)
        {
            m_line = line;
            m_correct = correct;
            m_bool = cond;
            m_expr = expr;
        }

        void assert_true() 
        {
            if (!m_bool) 
            {
                console::set_foreground_color(console_color::red);
                console::write_line_multi(
                    "Assert Failed, Lines: ", m_line, 
                    " ", m_correct,
                    " expression is: ", m_expr);
            }
        }

        ~tester()
        {
            console::reset();
        }

    private:
        std::string m_correct;
        std::string m_expr;
        bool m_bool;
        int m_line;

    }; // class tester

} // namespace leviathan::test

// exported macro

#define ASSERT_TRUE( boolean_expr ) ( leviathan::test::tester(boolean_expr, __LINE__, "expected true", STR(boolean_expr) ).assert_true() )
#define ASSERT_FALSE( boolean_expr ) ( leviathan::test::tester(not (boolean_expr), __LINE__, "expected false", STR(boolean_expr) ).assert_true() )
#define ASSERT_EQ( a, b ) ASSERT_TRUE(a == b)
#define ASSERT_NE( a, b ) ASSERT_TRUE(a != b)
#define ASSERT_LT( a, b ) ASSERT_TRUE(a < b)
#define ASSERT_LE( a, b ) ASSERT_TRUE(a <= b)
#define ASSERT_GE( a, b ) ASSERT_TRUE(a >= b)
#define ASSERT_GT( a, b ) ASSERT_TRUE(a > b)

#endif