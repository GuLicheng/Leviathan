#include <vector>
#include <ranges>
#include <random>
#include <fstream>
#include <algorithm>

namespace leviathan
{
    struct random_range
    {
        inline static std::random_device rd;

        int m_num = 1e8;
        int m_max = 100;

        random_range(int num = 1e8, int maxn = 100) : m_num(num), m_max(maxn) { }

        // static void PrintVec(const std::vector<int>& v)
        // {
        //     for (auto i : v) stream << i << ' ';
        //     stream << '\n';
        // }

        std::vector<std::string> read_context(const char* file = "a.txt")
        {
            std::fstream fs{file};
            if (!fs) return { };
            std::vector<std::string> ret;
            std::copy(std::istream_iterator<std::string>{fs}, std::istream_iterator<std::string>{}, std::back_inserter(ret));
            return ret;
        }

        std::vector<int> random_range_int()
        {
            auto random_generator = [&]() {
                return rd();
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
            std::vector<int> v; v.reserve(m_num);
            for (int i = 0; i < m_num; ++i) v.push_back(-i);
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




