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

/*
    
    INI_handler::load(const char* file, std::openmode) : read ini config file
    INI_handler::load(const string& file, std::openmode) : read ini config file
    INI_handler::load(const string_view file, std::openmode) : read ini config file
    INI_handler::get_items(): return all sections and entries
    INI_handler::get_sections(): return all sections
    INI_handler::get_entries(): return all entries
    INI_handler::getint(): 
    INI_handler::getfloat():
    INI_handler::getboolean():
    INI_handler::getstring():

    Comming soon:
    INI_handler::write(const char* file);
    INI_handler::write(const std::string& file);
    INI_handler::write(std::string_view file);
    follow method will be implemented by overloading operator[]
    INI_handler::add_sections
    INI_handler::add_key
    INI_handler::add_value
*/

#ifndef __INI_HPP__
#define __INI_HPP__

#include <lv_cpp/ranges/core.hpp>

#include <unordered_map>
#include <fstream>
#include <list>
#include <string>
#include <string_view>
#include <optional>
#include <cctype>
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
        const auto& key() const noexcept
        {
            return this->first;
        }
        auto& key() noexcept
        {
            return this->first;
        }
        auto& value() const noexcept
        {
            return this->second;
        }
        auto& value() noexcept
        {
            return this->second;
        }
    };

    // overload ostream
    std::ostream& operator<<(std::ostream& os, const entry& e)
    { 
        return os << '(' << e.first << ", " << e.second << ')'; 
    }  

    // list of key-value
    struct section_node
    {
        std::list<entry> ls;
    };

    // overload ostream
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
        INI_handler() : lines{0} 
        {    
        }

        INI_handler(const INI_handler&) = delete;
        INI_handler& operator=(const INI_handler&) = delete;
        ~INI_handler() = default;

        /*
         * load ini file
         * paras:
         *      file: the path of file
         *      openmode: the openmode for fstream
         * return:
         *      true if successful load file and parse all items
         *      false otherwise
         */
        bool load(const char* file, std::ios::openmode = std::ios::in | std::ios::binary);
        bool load(const std::string& file, std::ios::openmode mode = std::ios::in | std::ios::binary)
        { 
            return load(file.c_str(), mode); 
        }
        bool load(const std::string_view& file, std::ios::openmode mode = std::ios::in | std::ios::binary)
        {
            return load(file.data(), mode);
        }

        // erase all data within in this class
        // bool clear(section_node* node, entry* e);
        
        /*
         * whether the file is successfully loaded
         * return:
         *      true if load return true, otherwise false
         */
        bool is_loaded() const noexcept
        { 
            return in.is_open() && sections.size();
        }

        /*
         * get all sections from ini read
         * return:
         *      view of sections
         */
        auto get_sections() const noexcept
        {
            return sections | ::leviathan::views::keys;
        }

        /*
         * get all entries from ini read
         * return:
         *      view of entries
         */
        auto get_entries() const noexcept
        {
            return sections | ::leviathan::views::values;
        }

        /*
         * get all items from ini read
         * return:
         *      view of items
         */
        auto get_items() const noexcept
        {
            return sections | ::leviathan::views::all;
        }

        /*
         * whether INI_handler can work
         * return: 
         *      true if open and read all items successfully otherwise false
         */
        explicit operator bool() const noexcept
        {
            return static_cast<bool>(this->in) && is_loaded();
        }

        /*
         * parse a string to integer
         * paras:
         *      section_name: ...
         *      key_name: ...
         * return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to integer, otherwise nullpot
         */
        std::optional<int64_t> 
        getint(const std::string& section_name, const std::string& key_name) const;

        /*
         * parse a string to float
         * paras:
         *      section_name: ...
         *      key_name: ...
         * return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to float, otherwise nullpot
         */
        std::optional<double> 
        getfloat(const std::string& section_name, const std::string& key_name) const;

        /*
         * parse a string to boolean
         * paras:
         *      section_name: ...
         *      key_name: ...
         * return:
         *      optional if both section_name and key_name exist and the value can be 
         *      convert to boolean(true or false, ignore case), otherwise nullpot
         */
        std::optional<bool>
        getboolean(const std::string& section_name, const std::string& key_name) const;

        /*
         * get value item by section and key
         * paras:
         *      section_name: ...
         *      key_name: ...
         * return:
         *      optional if both section_name and key_name exist, otherwise nullopt
         */
        std::optional<std::string>
        getstring(const std::string& section_name, const std::string& key_name) const;

        // for debug
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
        std::string trim(const std::string& s) const noexcept;

        /* insert a section, if insert successfully, the node will pointer at section 
         * otherwise the log will memory this line
         * 
         * paras:
         *      node: the final address if insert successfully otherwise nullptr
         *      s: current string read from file
         *      line: the location of s in the file
         */
        void insert_section(section_node*& node, std::string s, int line);

    public:
        std::ifstream in;
        // I may change HashTable to LinkList some day
        std::unordered_map<std::string, section_node> sections;
        std::list<error_log> log; // save the line of error sections or entry
        int lines;
    }; //  end of class INI_handler


    bool INI_handler::load(const char* file, std::ios::openmode mode) 
    {
        in.open(file, mode);

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

            // the item in the left of '=' is key, right is value
            auto equal = str.find('=');

            // don't contain '='
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

    void INI_handler::insert_section(section_node*& node, std::string s, int line)
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

    std::string INI_handler::trim(const std::string& s) const noexcept
    {
        auto iter = std::find(s.begin(), s.end(), ';');
        std::ranges::subrange sub{s.cbegin(), iter};
        auto res_range = sub | ::leviathan::views::trim(::isspace);
        return {res_range.begin(), res_range.end()};
    }

    std::optional<int64_t> 
    INI_handler::getint(const std::string& section_name, const std::string& key_name) const  
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
        try
        {
            auto res = std::stod(*value);
            return res;
        }
        catch(...)
        {
            return {};
        }
        return {}; // useless
    }

    std::optional<double> 
    INI_handler::getfloat(const std::string& section_name, const std::string& key_name) const  
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
        try
        {
            auto res = std::stod(*value);
            return res;
        }
        catch(...)
        {
            return {};
        }
        return {}; // useless
    }

    std::optional<bool>
    INI_handler::getboolean(const std::string& section_name, const std::string& key_name) const
    {
        auto value = getstring(section_name, key_name);
        if (!value.has_value())
            return {};
    
        std::for_each(value->begin(), value->end(), [](char& c)
        {
            c = ::tolower(c);
        });
        if (value == "true")
            return true;
        if (value == "false")
            return false;
        return {};
    }

    std::optional<std::string>
    INI_handler::getstring(const std::string& section_name, const std::string& key_name) const
    {
        auto sec_iter = sections.find(section_name);
        if (sec_iter == sections.end())
            return {};
        auto value_iter = std::find_if(sec_iter->second.ls.begin(), sec_iter->second.ls.end(), [&](const auto& e)
        {
            return e.key() == key_name;
        });

        if (value_iter == sec_iter->second.ls.end())
            return {};
        return value_iter->second;
    }

} // namespace leviathan::INI




#endif