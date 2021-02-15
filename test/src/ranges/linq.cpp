#include <lv_cpp/linq/linq.hpp>
// #include <lv_cpp/io/console.hpp>
#include <cctype>
#include <vector>
#include <list>
#include <string>

using namespace leviathan::linq;

auto printer = [](auto x) { std::cout << x << ' '; };

struct integer
{
    int i;
    integer(int i) : i{i} 
    {
    }
    operator int() const
    { return i; }
    std::string name() const
    {
        return std::to_string(this->i) + "0";
    }
    bool operator<(const integer& rhs) const noexcept
    {
        return this->i > rhs.i;
    }
    // ~integer()
    // {
    //     std::cout << "Destoried\n";
    // }

};

void test1()
{
    std::list<integer> ls;
    for (int i = 0; i < 10; ++i)
        ls.emplace_back(i), ls.emplace_back(i);
    std::vector<integer> arr;
    arr.emplace_back(1);
    arr.emplace_back(2);
    arr.emplace_back(3);
    auto res = from(ls)  // [0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9]
        .reverse()      // [9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0]
        .where([](int x) { return x <= 3 || x >= 7; })
        .select([](integer x) { return x.name(); }) 
        .skip(1)  // 90 80 80 70 70 30 30 20 20 10 10 00 00
        .skip_while([](const std::string& str) { return str[0] >= '7'; }) // 30 30 20 20 10 10 00 00
        .take(5) // 30 30 20 20 10 (string)
        .select([](const std::string& str) { return std::stoi(str); }) // 30 30 20 20 10
        .take_while([](int x) { return x > 20; }) // 30 30
        .distinct()  // 30
        .concat(from(arr).select([](auto x) { return x.i * 10; })) // 30 10 20 30
        .distinct() // 30 10 20
        .for_each(printer)  // print above
        ;
}

void test2()
{
    std::cout << "\n===============================================\n";
    std::string str = "   123   ";
    std::string str1 = "456";
    from(str)
        .skip_while(::isspace).reverse().skip_while(::isspace).reverse()
        .concat(from(str1))
        .concat(from(std::string("789")))
        .for_each([](char c) { std::cout << c; }) // 123456789
        ;
    std::cout << "\n===============================================\n";
}

void test3()
{
    int src[] = {1,2,3,4,5,6,7,8};
    auto dst = from(src).where( [](int a) { return a % 2 == 1; })      // 1,3,5,7
                        .select([](int a) { return a * 2; })           // 2,6,10,14
                        .where( [](int a) { return a > 2 && a < 12; }) // 6,10
                        .for_each([](int x) {std::cout << x << ' '; })
                        ;
    std::cout << '\n';
    // dst type: std::vector<int>
    // dst items: 6,10
}

void test4()
{
    struct Message {
        std::string PhoneA;
        std::string PhoneB;
        std::string Text;
    };

    Message messages[] = {
        {"Anton","Troll","Hello, friend!"},
        {"Denis","Wride","OLOLO"},
        {"Anton","Papay","WTF?"},
        {"Denis","Maloy","How r u?"},
        {"Denis","Wride","Param-pareram!"},
    };

    int DenisUniqueContactCount =
        from(messages)
                    .where(   [](const Message & msg) { return msg.PhoneA == "Denis"; })
                    .select([](const Message & msg) { return msg.PhoneB; })
                    .distinct()
                    .count();
    std::cout << DenisUniqueContactCount << std::endl;
    // DenisUniqueContactCount == 2  
}


void test5()
{
    std::cout << "=======================\n";
    int arr[] = {1, 2, 3, 4, 5, 6};
    from(arr).take_while([](int x) { return x < 3; })
             .ordered_by([](int x) { return -x; })
             .for_each(printer)
             ;
    std::cout << '\n';
}

// void test6()
// {
//     std::vector<int> ls;
//     for (int i = 0; i < 2; ++i)
//         ls.emplace_back(i);

//     auto res = leviathan::linq::from(ls)
//         .ordered_by([](auto& x) { return integer(x); })
//         .for_each([](auto&& x) { std::cout << x << ' ';})
//         ;
// }

int main()
{
    test1();  // 30
    test2();  // 123456789
    test3();  // 6, 10
    test4();  // 2
    test5();  // 21
}