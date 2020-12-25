#ifndef _COLOR_COUT_HPP_
#define _COLOR_COUT_HPP_

#include <iostream>
template <typename CharT, typename Traits = std::char_traits<CharT>>
class color_ostream
{
    using self = color_ostream;
public:
    color_ostream(std::basic_ostream<CharT, Traits>* os = &std::cout)
        : m_os{os} { }

    ~color_ostream()
    {
        this->reset();
    }

    inline self& blue()
    {
         this->data() << "\033[34m";
         return *this;
    }

    inline self& black()
    {
        this->data() << "\033[30m";
        return *this; 
    }

    inline self& red()
    {
        this->data() << "\033[31m";
        return *this; 
    }

    inline self& green()
    {
        this->data() << "\033[32m";
        return *this;
    }

    inline self& yellow()
    {
        this->data() << "\033[33m";
        return *this;
    }

    inline self& magenta()
    {
        this->data() << "\033[35m";
        return *this;
    }

    inline self& cyan()
    {
        this->data() << "\033[36m";
        return *this;
    }

    inline self& white()
    {
        this->data() << "\033[37m";
        return *this;
    }

    inline self& bold()
    {
        this->data() << "\033[1m";
        return *this;
    }

    inline self& reset()
    {
        this->data() << "\033[0m";
        return *this;
    }

    inline self& high_light()
    {
        this->data() << "\033[1m";
        return *this;
    }

    inline self& reduce_light()
    {
        this->data() << "\033[2m";
        return *this;
    }

    inline self& italic()
    {
        this->data() << "\033[3m";
        return *this;
    }

    inline self& under_score()
    {
        this->data() << "\033[4m";
        return *this;
    }

    inline self& bar()
    {
        this->data() << "\033[9m";
        return *this;
    }

    template <typename T>
    color_ostream& operator<<(const T& val)
    {
        this->data() << val;
        return *this;
    }  

    inline std::ostream& data() const 
    {
        return *m_os;
    }

    inline self& clear_screen()
    {
        this->data() << "\033[2J";
        return *this;
    }

    inline self& shift_left(int offset)
    {
        const char c = offset + '0';
        this->data() << "\033[" << c << "D";
        return *this;
    }

    inline self& shift_right(int offset)
    {
        const char c = offset + '0';
        this->data() << "\033[" << c << "C";
        return *this;
    }

    inline self& shift_up(int offset)
    {
        const char c = offset + '0';
        this->data() << "\033[" << c << "A";
        return *this;   
    }

    inline self& shift_down(int offset)
    {
        const char c = offset + '0';
        this->data() << "\033[" << c << "B";
        return *this;   
    }


private:
    std::basic_ostream<CharT, Traits>* m_os;
};

// int main()
// {
//     color_ostream<char> os{&std::cout};
//     os.blue().italic().magenta().data() << "hello world";
//     os.shift_left(5);
//     os.yellow();
//     os.bold();
//     os.data() << "world!";
// }

/**
 *  \033[31m
 *  \033 ESC
 *  ESC[ :控制序列 
 *  第一个参数 31
 *  最终字节: m 图形再现
 * 
 */ 

/*
 \033[0m 关闭所有属性
\033[1m 高亮
\033[2m 亮度减半
\033[3m 斜体
\033[4m 下划线
\033[5m 闪烁
\033[6m 快闪
\033[7m 反显
\033[8m 消隐
\033[9m 中间一道横线
10-19 关于字体的
21-29 基本与1-9正好相反
30-37 设置前景色
40-47 设置背景色
30:黑
31:红
32:绿
33:黄
34:蓝色
35:紫色
36:深绿
37:白色
38 打开下划线,设置默认前景色
39 关闭下划线,设置默认前景色
40 黑色背景
41 红色背景
42 绿色背景
43 棕色背景
44 蓝色背景
45 品红背景
46 孔雀蓝背景
47 白色背景
48 不知道什么东西
49 设置默认背景色
50-89 没用
90-109 又是设置前景背景的，比之前的颜色浅
\033[nA 光标上移n行
\033[nB 光标下移n行
\033[nC 光标右移n行
\033[nD 光标左移n行
\033[y;xH设置光标位置
\033[2J 清屏
\033[K 清除从光标到行尾的内容
\033[s 保存光标位置
\033[u 恢复光标位置
\033[?25l 隐藏光标
\033[?25h 显示光标
*/ 

// #define BLACK   "\033[30m"      /* Black */
// #define RED     "\033[31m"      /* Red */
// #define GREEN   "\033[32m"      /* Green */
// #define YELLOW  "\033[33m"      /* Yellow */
// #define BLUE    "\033[34m"      /* Blue */
// #define MAGENTA "\033[35m"      /* Magenta */
// #define CYAN    "\033[36m"      /* Cyan */
// #define WHITE   "\033[37m"      /* White */
// #define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
// #define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
// #define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
// #define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
// #define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
// #define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
// #define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
// #define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#endif