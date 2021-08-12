#ifndef __TRIE_HPP__
#define __TRIE_HPP__

/*
 *    使用STL算法实现单词查找树类
 *      trie
 */

#include <iostream>
#include <optional>
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

template <typename T>
class trie {
public:
    template <typename Iter>
    void insert(Iter _Begin, Iter _End) {
        if (_Begin == _End) return;
        tries[*_Begin].insert(std::next(_Begin), _End);
    }

    template <typename C>
    void insert(const C& container) {
        insert(std::begin(container), std::end(container));
    }

    void insert(const std::initializer_list<T>& ls) {
        insert(std::begin(ls), std::end(ls));
    }

    void print(std::vector<T>& vec) const {
        if (tries.empty()) {
            std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>{ std::cout, ", " });
            std::cout << std::endl;
        }

        for (const auto& [key, val] : tries) {
            vec.emplace_back(key);
            val.print(vec);
            vec.pop_back();
        }
    }

    void print() const {
        std::vector<T> vec;
        print(vec);
    }

    template <typename Iter>
    std::optional<std::reference_wrapper<const trie>>
    subtrie(Iter first, Iter last) const {
        if (first == last) return std::ref(*this);
            
        auto found = tries.find(*first);
        if (found == std::end(tries)) return {};

        return found->second.subtrie(std::next(first), last);
    }

    template <typename C>
    auto subtrie(const C& c) {
        return subtrie(std::begin(c), std::end(c));
    }

private:
    std::map<T, trie> tries;
};

// static void prompt() {
//     std::cout << "Next input please: \n";
// }

// void test1();

// void test2();

// int main() {
//     test1();
//     test2();
// }


// void test1() {
//     trie<std::string> t;
//     t.insert({"hi", "how", "are", "you"});
//     t.insert({"hi", "i", "am", "great", "thanks"});
//     t.insert({"what", "are", "you", "doing"});
//     t.insert({"i", "am", "watching", "a", "movie"}); 
//     std::cout << "recorded sentences:\n";
//     t.print();
//     std::cout << "\npossible suggestions after \"hi\":\n";

//     if (auto st (t.subtrie(std::initializer_list<std::string>{"hi"}));
//         st) {
//         st->get().print();
//     }
// }

// void test2() {
//     trie<std::string> t;
//     std::fstream infile { "db.txt" };
//     for (std::string line; std::getline(infile, line);) {
//         std::istringstream iss { line };
//         t.insert(std::istream_iterator<std::string>{ iss }, { });
//     }
//     prompt();
//     for (std::string line; std::getline(std::cin, line);) {
//         std::istringstream iss{ line };

//         if (auto st = t.subtrie(std::istream_iterator<std::string>{ iss }, { }); st) {
//             std::cout << "Suggestions: \n";
//             st->get().print();
//         } else {
//             std::cout << "No suggestions found.\n";
//         }
//         std::cout << "--------------------------\n";
//         prompt();
//     }
// }

#endif