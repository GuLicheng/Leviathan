#ifndef __INI_HPP__
#define __INI_HPP__

#include <lv_cpp/ranges/core.hpp>

#include <unordered_map>
#include <fstream>
#include <list>
// #include <optional>

// for debug
#include <iterator> 
#include <iostream>
#include <string>
#include <string_view>


namespace leviathan::INI
{

    // entry, consist of key-value
    using entry = std::pair<std::string, std::string>;

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
        std::cout << s.ls.size() << std::endl;
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
                std::cout << "Sections is " << key << "\n";
                std::cout << value << std::endl;
            }

            std::cout << "Here is log\n";
            for (auto& l : log)
            {
                std::cout << l << std::endl;
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

        auto insert_section(std::string&& s)
        {
            return &(sections.try_emplace(std::move(s), section_node()).first->second);
            // return sections.emplace(std::move(s), section_node()).first;
        }

    public:
        std::ifstream in;
        std::unordered_map<std::string, section_node> sections;
        std::list<std::string> log; // save the line of error sections or entry
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
                node = insert_section(std::move(str));
                continue;
            }
            // parse entry

            // the item in the left of '=' is key, otherwise value
            auto equal = str.find('=');
            if (equal == str.npos)
            {
                // syntax error
                continue;
            }
            std::ranges::subrange left{str.begin(), str.begin() + equal};
            auto left_part = left | ::leviathan::views::trim_back(::isspace);
            auto key = std::string(left_part.begin(), left_part.end());

            std::ranges::subrange right{str.begin() + equal + 1, str.end()};
            auto right_part = right | ::leviathan::views::trim_front(::isspace);
            auto value = std::string(right_part.begin(), right_part.end());
            // std::cout << key << '-' << value << std::endl;
            
            if (!node)
            {
                // the entry must follow section or it will be ignored
                continue;
            }
            node->ls.emplace_back(std::move(key), std::move(value));
        }
     
        return true;
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