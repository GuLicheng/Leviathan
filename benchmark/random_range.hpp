#include <vector>
#include <ranges>
#include <random>
#include <fstream>
#include <algorithm>

#include <assert.h>

namespace leviathan
{
    struct random_range
    {
        inline static std::random_device rd;

        int m_num = 1e8;
        int m_max = 1e9;

        random_range(int num = 1e8, int maxn = 100) : m_num(num), m_max(maxn) { }

        static std::vector<std::string> read_context(const char* file = "a.txt")
        {
            std::fstream fs{file};
            if (!fs) return { };
            std::vector<std::string> ret;
            std::copy(std::istream_iterator<std::string>{fs}, std::istream_iterator<std::string>{}, std::back_inserter(ret));
            return ret;
        }

        std::vector<int> random_range_int()
        {
            auto random_generator = [this]() {
                return rd() % m_max;
            };
            std::vector<int> ret;
            std::generate_n(std::back_inserter(ret), m_num, random_generator);
            return ret;
        }

        std::vector<int> random_ascending()
        {
            std::vector<int> v; v.reserve(m_num);
            for (int i = 0; i < m_num; ++i) v.push_back(i);
            return v;
        }

        std::vector<int> zero()
        {
            return std::vector<int>(m_num, 0);
        }

        std::vector<int> random_descending()
        {
            auto v = random_ascending();
            std::ranges::reverse(v);
            return v;
        }

        std::vector<int> pipe_organ() 
        {
            std::vector<int> v; v.reserve(m_num);
            for (int i = 0; i < m_num/2; ++i) v.push_back(i);
            for (int i = m_num/2; i < m_num; ++i) v.push_back(m_num - i);
            return v;
        }

        std::vector<int> push_front_int() 
        {
            std::vector<int> v; v.reserve(m_num);
            for (int i = 1; i < m_num; ++i) v.push_back(i);
            v.push_back(0);
            return v;
        }

        std::vector<int> push_middle_int() 
        {
            std::vector<int> v; v.reserve(m_num);
            for (int i = 0; i < m_num; ++i) {
                if (i != m_num/2) v.push_back(i);
            }
            v.push_back(m_num/2);
            return v;
        }
    };
} // namespace leviathan

namespace leviathan
{
    inline constexpr auto default_num = 100'000; // DEFAULT_NUM x Catch::DataConfig::benchmarkSamples

    inline auto random_generator = random_range(default_num, default_num * 10);

    inline namespace insertion
    {
        inline auto ascending = random_generator.random_ascending();
        inline auto descending = random_generator.random_descending(); 
        inline auto random_int = random_generator.random_range_int(); 
    }

    inline namespace search
    {
        inline auto searching = random_generator.random_range_int();
    }

    inline namespace removing
    {
        inline auto remove = random_generator.random_range_int();
    }

    template <typename Set>
    auto random_insert_test()
    {
        Set s;
        for (auto val : insertion::random_int) s.insert(val);
        assert(s.size() <= default_num);
        return s.size();
    }

    template <typename Set>
    auto ascending_insert_test()
    {
        Set s;
        for (auto val : insertion::ascending) s.insert(val);
        assert(s.size() <= default_num);
        return s.size();
    }

    template <typename Set>
    auto descending_insert_test()
    {
        Set s;
        for (auto val : insertion::descending) s.insert(val);
        assert(s.size() <= default_num);
        return s.size();
    }

    template <typename Set>
    auto search_test(const Set& s)
    {
        assert(s.size() > 0);
        int cnt = 0;
        for (auto val : search::searching) 
            cnt += s.contains(val);
        return cnt;
    }

    template <typename Set>
    auto remove_test(Set& s)
    {
        assert(s.size() > 0);

        for (auto val : removing::remove) s.erase(val);

        return s.size();
    }

    template <typename... Sets>
    void random_insert(Sets&... s)
    {
        for (auto val : insertion::random_int) 
        {
            (s.insert(val), ...);   
        }
    }

    template <typename... Maps>
    void random_insert_map(Maps&... s)
    {
        bool is_all_empty = (s.empty() && ...); 
        assert(is_all_empty);
        for (auto val : insertion::random_int) 
        {
            (s.insert({val, std::to_string(val)}), ...);  
        }
    }

    template <typename Map>
    auto random_insert_test2()
    {
        Map s;
        for (auto val : insertion::random_int) s.insert({val, val});
        assert(s.size() <= default_num);
        return s.size();
    }

    template <typename Map>
    auto ascending_insert_test2()
    {
        Map s;
        for (auto val : insertion::ascending) s.insert({val, val});
        assert(s.size() <= default_num);
        return s.size();
    }

    template <typename Map>
    auto descending_insert_test2()
    {
        Map s;
        for (auto val : insertion::descending) s.insert({val, val});
        assert(s.size() <= default_num);
        return s.size();
    }
}

