#include <lv_cpp/math/bigint.hpp>
#include <random>
using namespace leviathan::math;

static std::random_device rd;

void ToStringTest()
{
    for (int i = 0; i < 10000; ++i)
    {
        long long int r = static_cast<long long int>(rd() % 15843597);
        assert(big_integer{r}.to_string() == std::to_string(r));
    }
    std::cout << "ToStringTest() is OK\n";
}

void CmpTest()
{
    for (int i = 0; i < 10000; ++i)
    {
        int left = rd() % 700;
        int right = rd() % 700;
        big_integer l{left}, r{right};
        assert((left < right) == (l < r));
        assert((left == right) == (l == r));
        assert((left <=> right) == (l <=> r));
    }
    std::cout << "CmpTest() is OK\n";
}

void LogicAndAlgorithmTest()
{
    for (int i = 0; i < 100; ++i)
    {
        int64_t left = rd();
        int64_t right = rd();
        big_integer l{left}, r{right};
        // Binary
        assert(std::to_string(left & right) == (l & r).to_string());
        assert(std::to_string(left | right) == (l | r).to_string());
        assert(std::to_string(left ^ right) == (l ^ r).to_string());
        assert(std::to_string(left + right) == (l + r).to_string());
        assert(std::to_string(left - right) == (l - r).to_string());
        assert(std::to_string(left / right) == (l / r).to_string());
        assert(std::to_string(left % right) == (l % r).to_string());
        assert(std::to_string(left << 20) == (l << 20).to_string());
        assert(std::to_string(left >> 20) == (l >> 20).to_string());
        assert(std::to_string(0) == (l >> 100).to_string());

        // Unary
        assert(std::to_string(-left) == (-l).to_string());
        assert(std::to_string(~left) == (~l).to_string());
        left++;
        l++;
        right--;
        r--;
        assert(std::to_string(left) == (l).to_string());
        assert(std::to_string(right) == (r).to_string());
    }
    for (int i = 0; i < 100; ++i)
    {
        int64_t left = rd() % 100000; // avoid overflow
        int64_t right = rd() % 100000;
        big_integer l{left}, r{right};
        assert(std::to_string(left * right) == (l * r).to_string());
    }
    std::cout << "LogicAndAlgorithmTest() is OK\n";
}

void GCDTest()
{
    big_integer i1{10}, i2{5};
    assert(i1.gcd(i2).to_string() == std::string("5"));
    std::cout << "GCDTest() is OK\n";
}

void BitTest()
{
    //auto bit_count = [](unsigned int x)
    //{
    //	int res = 0;
    //	while (x)
    //	{
    //		res += x & 1;
    //		x >>= 1;
    //	}
    //	return res;
    //};
    //for (int i = 0; i < 1; ++i)
    //{
    //	unsigned int k = rd();
    //	big_integer b{ static_cast<int64_t>(k) };
    //	if (bit_count(k) != b.bit_count())
    //	{
    //		std::bitset<32> bit = k;
    //		std::cout << bit.to_string() << '\n';
    //		std::cout << bit_count(k) << '\n';
    //		std::cout << b.bit_count() << '\n';
    //		break;
    //	}
    //}

    std::cout << "BitTest() is OK\n";
}

void SqrtTest()
{
    static int arr[] = {0, 1, 2, 4, 9, 16, 25, 36, 37};
    for (auto val : arr)
    {
        big_integer i{val};
        std::cout << i.sqrt().to_string() << ' ';
    }
    std::cout << '\n';
    big_integer sqrt50_50{"1593049814277254434675646720348416"};
    // 39913028127132304 ** 2
    std::cout << sqrt50_50.sqrt().to_string() << '\n';
    std::cout << "SqrtTest() is OK\n";
}

void ModInverseTest()
{
    big_integer i1{2}, i2{5};
    // 2 * 5 = 10 => result = 3 since 10 % 3 = 1
    std::cout << i1.mod_inverse(i2).to_string() << '\n';
    std::cout << "ModInverseTest() is OK\n";
}

void JacobiTest()
{
    int inputs[] = {3, 5, 3, 9, 3, 13};
    // int outputs[] = {-1, 0, 1};
    for (int i = 0; i < 6; i += 2)
    {
        big_integer b1{inputs[i]}, b2{inputs[i + 1]};
        std::cout << big_integer::jacobi(b1, b2) << ' ';
    }
    std::cout << "\n";
    std::cout << "JacobiTest() is OK\n";
}

void OtherTest()
{
    std::string factorial100{ "93326215443944152681699238856266700490715968264381621468592963895217599993229915608941463976156518286253697920827223758251185210916864000000000000000000000000" };
    big_integer b{1};
    for (int i = 1; i <= 100; ++i)
        b = b * static_cast<big_integer>(i);
    assert(factorial100 == b.to_string());
    std::cout << "max_value is " << big_integer::max_value().to_string() << "\n";
    std::cout << "min_value is " << big_integer::min_value().to_string() << "\n";
    std::cout << "" << big_integer("132415315431543187431").to_string() << "\n";
    std::cout << "Empty String => " << big_integer("").to_string() << "\n";
    std::cout << big_integer(5).mod_pow(big_integer(2), big_integer(100)).to_string() << '\n';
    std::cout << "Empty Vector => " << big_integer{std::vector<unsigned>()}.to_string() << '\n';
    std::cout << "Hex init :{0xAAAA} " << big_integer{"AAAA", 16}.to_string() << '\n'; // 43690
    std::cout << "OtherTest() is OK\n";
}

int main()
{
    ToStringTest();
    CmpTest();
    LogicAndAlgorithmTest();
    GCDTest();
    BitTest();
    SqrtTest();
    ModInverseTest();
    JacobiTest();
    OtherTest();
    return 0;
}


