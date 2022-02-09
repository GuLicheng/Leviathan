
template <typename T1,  typename T2>
T1 minus(T2 x, T2 y)
{
    return x - y;
}


int main(int argc, char const *argv[])
{
    auto h = minus<float>(10, 6);
    return 0;
}


