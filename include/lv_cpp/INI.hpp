#ifndef __INI_HPP__
#define __INI_HPP__

#include <lv_cpp/ranges/core.hpp>

#include <unordered_map>
#include <fstream>
#include <list>
#include <optional>

// for debug
#include <iterator> 
#include <iostream>
#include <string>
#include <string_view>
#include <memory>

namespace leviathan::INI
{

    class Log 
    {
    protected:
        int line;
        std::string contend;
        std::string error_info;
    public:
        Log(int line, std::string contend, std::string error_info) 
            : line{line}, contend{std::move(contend)}, error_info{std::move(error_info)} { }
        
        void report() const 
        {
            std::cout << "Line: " << line << 
                ", error infomation:  " << error_info << 
                " Seeing here:" << contend << std::endl;  
        }

        ~Log() { }
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
        std::list<Log> log; // save the line of error sections or entry
        int lines;
    }; //  end of class INI_handler


    bool INI_handler::load(const char* file) 
    {
        in.open(file, ::std::ios::binary);

        // file not exist
        if (!in.is_open())
            return false;

        std::string line;
        // std::unordered_map<std::string, section_node>::iterator node;
        section_node* node = nullptr;
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
            
            if (!node)
            {
                // the entry must follow section or it will be ignored
                // another better way is prescan file and find first section
                continue;
            }
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