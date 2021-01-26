/**

 * !!! 我们的目的是能够正确读取红色警戒系列中的ini配置文件

 * 说明：ini配置文件读写

 * 1、支持;注释符号。// 非必须不支持 # 注释符号

 * 2、不支持带引号'或"成对匹配的字符串，否则无法正确读取相关词条，比如 name = stalin's fist

 * 3、如果无section，将读取不到任何信息，但section可以为空。

 * 4、不支持10、16、8进制数，0x开头为16进制数，0开头为8进制，但是我们日后可能会为此提供专门的接口。

 * 5、支持section、entry或=号前后带空格。

 * 6、按行读取，不接受换行。如何您有更高的要求，请使用json文件。

 * 7、区分section、key大小写。

 * 8、不支持任何修改ini操作，因为我们只是读取文件，配置文件建议在源文件上手动修改，实际上大家在制作mod的时候都是这么做的。

 * 9、读取文件的时候不会对section和entry内容部分做任何检查， 但是可能会额外设置了一个接口会对读取后的结果进行检查，如何需要的话可以使用。

 * 10、每个key只有一个value，如果当前key已经存在且包含value那么将会覆盖之前的value

 * 15、由于不知道+=的含义，对此不做处理， 比如
            Line1: 19	=	THOR
            Line2: +=STHOR

 */

#ifndef __INI_HPP__
#define __INI_HPP__

#include <lv_cpp/ranges/core.hpp>

#include <unordered_map>
#include <fstream>
#include <list>
#include <string>

// for debug
#include <iostream>

namespace leviathan::INI
{

    class error_log 
    {
    protected:
        int line;
        std::string contend;
        std::string error_info;
    public:
        error_log(int line, std::string contend, std::string error_info) 
            : line{line}, contend{std::move(contend)}, error_info{std::move(error_info)} { }
        
        void report() const 
        {
            std::cout << "Line: " << line << 
                ", error infomation:  " << error_info << 
                " Seeing here:" << contend << std::endl;  
        }

        ~error_log() { }
    };

    // entry, consist of key-value
    struct entry : public std::pair<std::string, std::string>
    {
        using std::pair<std::string, std::string>::pair;
        using std::pair<std::string, std::string>::operator=;
    };

    std::ostream& operator<<(std::ostream& os, const entry& e)
    { 
        return os << '(' << e.first << ", " << e.second << ')'; 
    }  

    // list of key-value
    struct section_node
    {
        std::list<entry> ls;
    };

    std::ostream& operator<<(std::ostream& os, const section_node& s)
    {
        std::cout << "The size of current section node is :" << s.ls.size() << std::endl;
        for (auto& e : s.ls)
        {
            std::cout << e << std::endl;
        }
        return os;
    }

    class INI_handler
    {
    public:
        INI_handler() : lines{0} { }
        ~INI_handler() = default;

        INI_handler(const INI_handler&) = delete;
        INI_handler& operator=(const INI_handler&) = delete;

        // Fetch and store INI data
        bool load(const char* file);
        bool load(const std::string& file);

        // erase all data within in this class
        // bool clear(section_node* node, entry* e);
        
        // int line_count(const section_node* section) const;
        bool is_loaded() const noexcept;
        int size() const noexcept;
        
        int section_count() const noexcept;
        // int entry_count() const noexcept;

        // std::optional<section_node&> operator[](const std::string&);


        explicit operator bool() const noexcept
        { return static_cast<bool>(this->in); }

        void show()
        {
            for (auto& [key, value] : sections)
            {
                std::cout << key << "\n";
                std::cout << value << std::endl;
            }

            std::cout << "Here is log\n";
            for (auto&& l : log)
            {
                l.report();
            }
        }

    private:
        // remove all ; and blank
        std::string trim(const std::string& s) const noexcept
        {
            auto iter = std::find(s.begin(), s.end(), ';');
            std::ranges::subrange sub{s.cbegin(), iter};
            auto res_range = sub | ::leviathan::views::trim(::isspace);
            return {res_range.begin(), res_range.end()};
        }

        void insert_section(section_node*& node, std::string&& s, int line)
        {
            // check whether the sections only contains one '[' and ']'
            // if not match, not change node
            if (s.find_first_of(']') != s.size() - 1 || s.find_last_of('[') != 0)
            {
                log.emplace_back(line, std::move(s), "more than one [ or ].");
                return;
            }
            node = &(sections.try_emplace(s.substr(1, s.size() - 2), section_node()).first->second);
        }

    public:
        std::ifstream in;
        // I may change HashTable to LinkList some day
        std::unordered_map<std::string, section_node> sections;
        std::list<error_log> log; // save the line of error sections or entry
        int lines;
    }; //  end of class INI_handler


    bool INI_handler::load(const char* file) 
    {
        in.open(file, ::std::ios::binary);

        // file not exist
        if (!in.is_open())
            return false;

        section_node* node = nullptr;
        std::string line;

        // prescan and get first section
        while (!in.eof())
        {
            std::getline(in, line);
            ++lines;
            auto str = trim(line);
            if (str.empty())
                continue;
            if (str[0] == '[')
            {
                insert_section(node, std::move(str), lines);
                if (node) break;
                else continue;
            }
        }

        // parse
        while (!in.eof())
        {
            std::getline(in, line);
            ++lines;
            auto str = trim(line);

            if (str.empty())
                continue;

            // handle section node
            
            if (str[0] == '[')
            {
                // parse sections
                insert_section(node, std::move(str), lines);
                continue;
            }
            // parse entry

            // the item in the left of '=' is key, otherwise value
            auto equal = str.find('=');

            // don't contain '=
            if (equal == str.npos)
            {
                // syntax error
                log.emplace_back(lines, std::move(str), "Not found =.");
                continue;
            }

            // start with '=' such as =foobar
            if (equal == 0)
            {
                log.emplace_back(lines, std::move(str), "Not found key.");
                continue;
            }

            // end with '=' such as foobar=
            if (equal == str.size() - 1)
            {
                log.emplace_back(lines, std::move(str), "Not found Value.");
                continue;
            }

            // trim key
            std::ranges::subrange left{str.begin(), str.begin() + equal};
            auto left_part = left | ::leviathan::views::trim_back(::isspace);
            auto key = std::string(left_part.begin(), left_part.end());

            // trim value
            std::ranges::subrange right{str.begin() + equal + 1, str.end()};
            auto right_part = right | ::leviathan::views::trim_front(::isspace);
            auto value = std::string(right_part.begin(), right_part.end());
            
            // save entry
            node->ls.emplace_back(std::move(key), std::move(value));
        }
     
        return true;
    }

    bool INI_handler::load(const std::string& file)
    {
        return load(file.c_str());
    }

    int INI_handler::section_count() const noexcept
    {
        return this->sections.size();
    }

    bool INI_handler::is_loaded() const noexcept
    {
        return this->in.is_open();
    }


} // namespace leviathan::INI




#endif