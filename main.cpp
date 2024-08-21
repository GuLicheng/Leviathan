#include <filesystem>
#include <leviathan/print.hpp>
#include <ranges>
#include <vector>
#include <string>
#include <generator>
#include <leviathan/config_parser/json/json.hpp>
#include <leviathan/config_parser/toml/toml.hpp>
#include <leviathan/config_parser/value_cast.hpp>
#include <leviathan/ranges/action.hpp>
#include <algorithm>

namespace json = leviathan::json;
namespace toml = leviathan::toml;

std::generator<std::string> RecurseFile(std::string dir)
{
    for (const auto& file : std::filesystem::directory_iterator(dir))
    {
        if (file.is_directory())
        {
            for (const auto& subfile : RecurseFile(file.path().string()))
            {
                co_yield subfile;
            }
        }
        else
        {
            co_yield file.path().string();
        }
    }
}

inline constexpr auto EndWith = [](const char* suffix) 
{
    return [=](const auto& entry) 
    {
        return entry.ends_with(suffix);
    };
};

inline constexpr auto RemoveExtension = [](const std::string& name)
{
    return name.substr(0, name.rfind('.'));
};

auto Read(const char* dir, const char* extension)
{
    return RecurseFile(dir) 
         | std::views::filter(EndWith(extension))
         | std::views::transform(RemoveExtension)
         | std::ranges::to<std::vector<std::string>>();
}

bool CheckType(std::string_view type, const toml::value& target)
{
    if (type == "integer")
    {
        return target.is<toml::integer>();
    }
    else if (type == "bool")
    {
        return target.is<toml::boolean>();
    } 
    else if (type == "float")
    {
        return target.is<toml::floating>();
    }
    else
    {
        throw std::runtime_error("Unknown type.");
    }
}

bool CompareJsonAndTomlValue(const json::value& jval, const toml::value& tval)
{
    auto type = jval.as<json::object>().find("type")->second.as<json::string>();
    bool ok1 = CheckType(type, tval);

    if (!ok1)
    {
        Console::WriteLine("Type parse error.");
        return false;
    }

    auto source = jval.as<json::object>().find("value")->second.as<json::string>();

    auto j = leviathan::config::detail::toml2json()(tval);

    auto x1 = std::format("{}", source);
    auto x2 = std::format("{}", j);

    // FIXME:
    // Compare string directly may not suitable.
    // Nan should ignore sign.
    if (x1 != x2)
    {
        Console::WriteLine("x1 = {} and x2 = {}", x1, x2);
        return false;
    }

    return true;
}

struct JsonMatcher
{
    bool Match(const json::value& root, const std::vector<std::string>& keys, const toml::value& target)
    {
        const json::value* cur = &root;

        for (const auto& key : keys)
        {
            cur = &(cur->as<json::object>().find(key)->second);
        }

        return CompareJsonAndTomlValue(*cur, target);
    }
};

struct TomlExtractor
{
    using path = std::vector<std::string>;
    using value = toml::value;

    std::vector<std::pair<path, const value*>> paths;

    void Dfs(const toml::value& x, path& cur)
    {
        if (!x.is<toml::table>() || (x.is<toml::table>() && x.as<toml::table>().empty()))
        {
            paths.emplace_back(cur, &x);
            return;
        }

        assert(x.is<toml::table>());
        for (auto& t = x.as<toml::table>(); const auto& [key, value] : t)
        {
            cur.emplace_back(key);
            Dfs(value, cur);
            cur.pop_back();
        }
    }

public:

    void Extract(const toml::value& x)
    {
        // Console::WriteLine(leviathan::config::detail::toml2json()(x));
        path p;
        Dfs(x, p);
    }

    void CompareToJson(const json::value& root, std::string filename)
    {
        JsonMatcher matcher;

        for (const auto& path_v : paths)
        {
            const auto& [keys, val] = path_v;
            if (!matcher.Match(root, keys, *val))
            {
                Console::WriteLine("Error file: {}", filename);
            }
        }
    }
};

void TestFiles(const char* dir)
{
    auto files = Read(dir, ".toml");

    for (auto file : files)
    {
        auto json_file = file + ".json";
        auto toml_file = file + ".toml";

        // Console::WriteLine("Compare file:\n [{}|{}]", json_file, toml_file);

        TomlExtractor extractor;
        auto tr = toml::load(toml_file.c_str());
        auto jr = json::load(json_file.c_str());
        extractor.Extract(tr);
        extractor.CompareToJson(jr, file);
    }
}

// --------------------------------------------------------------------

class Recorder
{
    std::vector<std::string> success;
    std::vector<std::string> failure;

public:

    void Success(std::string filename)
    {
        success.emplace_back(std::move(filename));
    }

    void Failure(std::string filename)
    {
        failure.emplace_back(filename);
    }

    void ReportFailure()
    {
        // Console::WriteLine("---{}--- failed!", failure);
    }
};

class Tester
{
    std::vector<std::string> files;
    Recorder recorder;

public:

    Tester(const char* dir) 
    {
        files = Read(dir, ".toml");
    }

    void ErrorFile(const char* file)
    {
        std::erase_if(files, [=](const std::string& filename)
        {
            return filename.ends_with(file);
        });
    }

    void TestFile(std::string filename)
    {
        filename += ".toml";

        // Console::WriteLine("Parsing file: {}", filename);
        try
        {
            auto dict = toml::load(filename.c_str());
            recorder.Success(filename);
            // Console::WriteLine("{} ok", filename);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            recorder.Failure(filename);
            Console::WriteLine("{} failed", filename);
        }
        
    }

    void TestFiles()
    {
        for (const auto& file : files)
        {
            TestFile(file);
        }
    }

    void ReportFailure()
    {
        recorder.ReportFailure();
    }

};

void TestToml()
{
    const char* dirs[] = {
        R"(D:\code\toml-test\tests\valid\array)",
        R"(D:\code\toml-test\tests\valid\bool)",
        R"(D:\code\toml-test\tests\valid\comment)",
        R"(D:\code\toml-test\tests\valid\datetime)",
        R"(D:\code\toml-test\tests\valid\float)",
        R"(D:\code\toml-test\tests\valid\inline-table)",
        R"(D:\code\toml-test\tests\valid\integer)",
    };
    
    for (auto dir : dirs) 
    {
        Tester tester(dir);
        tester.ErrorFile("newline");
        tester.TestFiles();
        tester.ReportFailure();
    }
}

void DebugFile(const char* file)
{
    auto t = toml::load(file);
    auto j = leviathan::config::detail::toml2json()(t);
    Console::WriteLine(j);
}

int main(int argc, char const *argv[])
{
    // TestToml();

    // DebugFile(R"(D:\code\toml-test\tests\valid\array\string-with-comma-2.toml)");

    DebugFile("../a.toml");

    return 0;
}
