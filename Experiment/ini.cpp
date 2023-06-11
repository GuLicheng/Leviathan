#include <ranges>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>


inline constexpr auto TrimBlank = [](std::string_view line) {
    auto first = std::ranges::find_if_not(line, ::isspace);
    auto last = std::ranges::find_if_not(line 
        | std::views::drop(std::ranges::distance(line.begin(), first)) 
        | std::views::reverse, ::isspace).base();
    return std::string_view(first, last);
};

inline constexpr auto GetSectionOrEntryContext = [](auto line) {

    // for empty line or line just with comments, return "".

    auto str = std::string_view(line);

    auto end_of_line = std::ranges::find(str, ';');  // find comment

    auto new_line = std::string_view(std::ranges::begin(str), end_of_line);

    auto line_after_trim = TrimBlank(new_line);

    auto result = std::string_view(line_after_trim.begin(), line_after_trim.end());

    return result;
};

inline constexpr auto IsSection = [](auto str) {
    return str.front() == '[';
};

inline constexpr auto IsEntry = [](auto str) {
    return !IsSection(str);
};

inline constexpr auto ChunkSectionAndEntries = [](auto l, auto r) {
    return (IsSection(l) && IsEntr  y(r)) || (IsEntry(l) && IsEntry(r));
};

inline constexpr auto SplitEntryToKeyValue = [](auto line) {

    assert(line.front() != '='); // only allow empty value

    auto kv = line | std::views::split('=');

    auto iter = kv.begin();

    auto key = std::string_view(*iter);

    std::ranges::advance(iter, 1, kv.end());

    return std::make_pair(key, iter == kv.end() ? "" : std::string_view(*iter));

};

inline constexpr auto MapEntriesToDict = [](auto lines) {
    std::unordered_map<std::string_view, std::string_view> dict;
    std::ranges::for_each(lines, [&](auto line) {
        auto [k, v] = SplitEntryToKeyValue(line);
        dict[k] = v;
    });
    return dict;
};

inline constexpr auto ParseOneSectionAndEntriesChunkToResultValueType = [](auto lines) {
    auto section = *lines.begin();
    auto entries = MapEntriesToDict(lines | std::views::drop(1));
    return std::make_pair(
        section.substr(1, section.size() - 2), 
        std::move(entries));
};


class SimpleInIReader
{

    friend class SimpleInIWriter;

public:

    SimpleInIReader(std::string_view path)
    {
        std::ifstream ifs { path.data(), std::ios::binary };
        context = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        TryParse();
    }

    const auto& operator[](std::string_view section) 
    {
        return result[section];
    }

private:

    void TryParse()
    {
        auto sections = context 
            | std::views::split('\n')
            | std::views::transform(GetSectionOrEntryContext)
            | std::views::filter(std::ranges::size)
            | std::views::chunk_by(ChunkSectionAndEntries)  
            | std::views::transform(ParseOneSectionAndEntriesChunkToResultValueType);
        //  | std::views::to<...>(); 

        std::ranges::copy(sections, std::inserter(result, result.end()));

    }

    std::string context;
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::string_view>> result;

};

class SimpleInIWriter
{
public:

    explicit SimpleInIWriter(SimpleInIReader& reader)
    {
        CopyResultFromReader(reader);
    }

    SimpleInIWriter() = default;

    void Write(std::ostream& os) const
    {
        for (const auto& [section, kv]  : result)
        {
            os << '[' << section << "]\n";
            for (auto [k, v] : kv)
                os << k << "=" << v << "\n";
        }
    }

    auto& operator[](std::string section) 
    {
        return result[std::move(section)];
    }


private:

    void CopyResultFromReader(SimpleInIReader& reader)
    {
        for (const auto& [section, entries] : reader.result)
        {
            std::unordered_map<std::string, std::string> dict;
            for (const auto& [key, value] : entries)
            {
                dict[std::string(key)] = std::string(value);
            }
            result[std::string(section)] = std::move(dict);
        }
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> result;
};



int main()
{
    SimpleInIReader reader { "a.ini" };

    SimpleInIWriter writer { reader };

    writer["Boris"]["Cost"] = "1500";

    writer.Write(std::cout);
}
