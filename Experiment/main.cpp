
#pragma once

#include <lv_cpp/meta/template_info.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <functional>
#include <string>
#include <any>
#include <utility>
#include <syncstream>

template <typename T>
struct referece_traits : std::type_identity<T> { };

template <typename T>
struct referece_traits<std::reference_wrapper<T>> : std::type_identity<T&> { };

class ioc_container
{
    std::map<std::string_view, std::any> m_map; // vector may faster if there are few type in the map

public:

    template <typename BaseClass, typename DerivedClass, typename... Args>
    void register_type()
    {

        static_assert(std::is_base_of_v<BaseClass, DerivedClass>);
        static_assert(std::is_constructible_v<DerivedClass, Args...>);
        static_assert(std::is_same_v<BaseClass, std::remove_cvref_t<BaseClass>>);
        static_assert(std::is_same_v<DerivedClass, std::remove_cvref_t<DerivedClass>>);

        std::string_view name = TypeInfo(BaseClass);
        std::function<BaseClass*(Args...)> fn = [](Args... args)
        {
            return new DerivedClass(args...);
        };
        m_map[name] = fn;
    }


    template <typename BaseClass, typename... Args>
    std::unique_ptr<BaseClass> reslove(Args... args)
    {
        auto iter = m_map.find(TypeInfo(BaseClass));
        if (iter == m_map.end()) 
            return nullptr;

        using fn_type = std::function<BaseClass*(typename referece_traits<Args>::type...)>;
        auto& fn = std::any_cast<fn_type&>(iter->second);
        auto* p = fn(std::move(args)...);
        return std::unique_ptr<BaseClass>(p);
    }

};

struct IStream
{
    virtual ~IStream() { }
    virtual void Report(std::string_view = "") = 0;
};

struct ConsoleStream : IStream
{
    ConsoleStream(std::ostream& os) : m_os{ os } { }
    ~ConsoleStream() { std::cout << "ConsoleStream Destory\n"; }

    void Report(std::string_view info = "ConsoleStream") override
    { m_os << info; }

    std::ostream& m_os;
};

struct FileStream : IStream
{
    FileStream(const std::string& filepath) : m_fs{ } { m_fs.open(filepath, std::ios_base::binary | std::ios_base::out); }
    ~FileStream() { std::cout << "FileStream Destory\n"; }

    void Report(std::string_view info = "FileStream") override
    { m_fs << info; }
    
    std::ofstream m_fs;
};

int main()
{
    ioc_container c;
    c.register_type<IStream, ConsoleStream, std::ostream&>();
    c.reslove<IStream>(std::ref(std::cout))->Report("Console.WriteLn");
    c.register_type<IStream, FileStream, std::string>();
    std::string file = "./Hello.txt";
    c.reslove<IStream>(std::move(file))->Report("File");
}