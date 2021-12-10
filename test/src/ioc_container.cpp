#include <lv_cpp/ioc_container.hpp>

#include <string>
#include <iostream>
#include <fstream>


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
    FileStream(const std::string& filepath) : m_fs{ } { m_fs.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::app); }
    ~FileStream() { std::cout << "FileStream Destory\n"; }

    void Report(std::string_view info = "FileStream") override
    { m_fs << info; }
    
    std::ofstream m_fs;
};

int main()
{
    ioc_container c;
    c.register_type<IStream, ConsoleStream, std::ostream&>();
    c.reslove<IStream>(std::ref(std::cout))->Report("Console.WriteLn\n");
    c.register_type<IStream, FileStream, std::string>();
    std::string file = "./Hello.txt";
    c.reslove<IStream>(std::move(file))->Report("FileStream.WriteLn(\"This is IOCContainer\")\n");
}